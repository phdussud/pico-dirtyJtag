-- DirtyJTAG dissector with VCD logger

local dirtyjtag = Proto("dirtyjtag", "DirtyJTAG Protocol")

-- Protocol fields
local f_command = ProtoField.uint8("dirtyjtag.command", "Command", base.HEX)
local f_payload = ProtoField.bytes("dirtyjtag.payload", "Payload")
local f_signal  = ProtoField.uint8("dirtyjtag.signal", "Signal", base.HEX)

dirtyjtag.fields = { f_command, f_payload, f_signal }

-- Command names
local cmd_names = {
    [0x0] = "CMD_STOP",
    [0x1] = "CMD_INFO",
    [0x2] = "CMD_FREQ",
    [0x3] = "CMD_XFER",
    [0x4] = "CMD_SETSIG",
    [0x5] = "CMD_GETSIG",
    [0x6] = "CMD_CLK",
    [0x7] = "CMD_SETVOLTAGE",
    [0x8] = "CMD_GOTOBOOTLOADER"
}

-- Logger state
local events = {}
local pending_out = {}
--local processed_frames = {}

local vcd_ids = { CLK="!", TDI="\"", TMS="#", TRST="$", SRST="%", TDO="&" }

-- === Logger functions ===
local function add_event(ev)
    table.insert(events, ev)
end

local function merge_in(ev_in)
    local tdo_bits = ev_in.tdo
    local idx = 1

    while #pending_out > 0 do
        local out_ev = pending_out[1]
        local needed = 0
        table.remove(pending_out, 1)
        if idx <= #tdo_bits then
            if out_ev.cmd == 0x3 and not out_ev.noread then
                needed = out_ev.cycles  -- bit_len
                out_ev.tdo = {}
                for c = 1, needed do
                    if idx > #tdo_bits then break end
                    table.insert(out_ev.tdo, tdo_bits[idx])
                    idx = idx + 1
                end

            elseif (out_ev.cmd == 0x6 and out_ev.readout)
                or (out_ev.cmd == 0x5) then
                -- Only one TDO bit: derived from the whole byte
                out_ev.tdo = {}
                if idx <= #tdo_bits then
                    -- Collapse the next 8 bits into a single value
                    local val = 0
                    for j = 0,7 do
                        if idx+j <= #tdo_bits and tdo_bits[idx+j] == 1 then
                            val = 1
                            break
                        end
                    end
                    table.insert(out_ev.tdo, val)
                    idx = idx + 8  -- consume the whole byte
                end
            else
                -- No TDO expected
            end

            -- After consuming the exact number of bits, discard any padding
            -- so the next OUT starts on a fresh byte boundary
            local pad = (needed % 8)
            if pad ~= 0 then
                idx = idx + (8 - pad)
            end
        end
        add_event(out_ev)
    end
end
local function tobin(n, len)
    local s = ""
    for i = len - 1 , 0, -1 do
        s = s .. (bit.band(bit.rshift(n, i), 1) == 1 and "1" or "0")
    end
    return s
end


local function write_vcd(filename)
    local f = assert(io.open(filename, "w"))
    f:write("$date\n   " .. os.date() .. "\n$end\n")
    f:write("$version\n   DirtyJTAG VCD Logger\n$end\n")
    f:write("$timescale 1ns $end\n")
    f:write("$scope module jtag $end\n")

    f:write("$var wire 16 ' SEQ $end\n")
    f:write("$var wire 8 A CMDNO $end\n")
    f:write("$var wire 8 B CMD $end\n")
    for name,id in pairs(vcd_ids) do
        f:write(string.format("$var wire 1 %s %s $end\n", id, name))
    end
    f:write("$upscope $end\n$enddefinitions $end\n")

    local time = 0
    for _, ev in ipairs(events) do
        f:write(string.format("$comment Transaction #%d %s %s $end\n",
            ev.seq, ev.dir, cmd_names[ev.cmd] or ""))
        f:write("#" .. time .. "\n")
        f:write("b" .. tobin(ev.seq, 16) .. " '\n")
        f:write("b" .. tobin(ev.txn, 8) .. " A\n")
        f:write("b" .. tobin(ev.cmd, 8) .. " B\n")

        if ev.cmd == 0x04 then
            -- SETSIG drives static signals immediately
            f:write("#" .. time .. "\n")
            if ev.mask and ev.values then
                if bit.band(ev.mask, 0x02) ~= 0 then
                    f:write((bit.band(ev.values,0x02)~=0 and "1" or "0") .. vcd_ids.CLK .. "\n")
                end
                if bit.band(ev.mask, 0x04) ~= 0 then
                    f:write((bit.band(ev.values,0x04)~=0 and "1" or "0") .. vcd_ids.TDI .. "\n")
                end
                if bit.band(ev.mask, 0x08) ~= 0 then
                    f:write((bit.band(ev.values,0x08)~=0 and "1" or "0") .. vcd_ids.TDO .. "\n")
                end
                if bit.band(ev.mask, 0x10) ~= 0 then
                    f:write((bit.band(ev.values,0x10)~=0 and "1" or "0") .. vcd_ids.TMS .. "\n")
                end
                if bit.band(ev.mask, 0x20) ~= 0 then
                    f:write((bit.band(ev.values,0x20)~=0 and "1" or "0") .. vcd_ids.TRST .. "\n")
                end
                if bit.band(ev.mask, 0x40) ~= 0 then
                    f:write((bit.band(ev.values,0x40)~=0 and "1" or "0") .. vcd_ids.SRST .. "\n")
                end
            end
            time = time + 50

        elseif ev.cmd == 0x06 then
            -- CMDCLK cycles
            local cycles = ev.cycles or 1
            local readout = ev.readout or false

            for c = 1, cycles do
                -- drive TDI/TMS at start of cycle
                f:write("#" .. (time +25) .. "\n")
                f:write((ev.tdi and "1" or "0") .. vcd_ids.TDI .. "\n")
                f:write((ev.tms and "1" or "0") .. vcd_ids.TMS .. "\n")

                -- rising edge
                f:write("#" .. (time + 50) .. "\n1" .. vcd_ids.CLK .. "\n")

                -- only log TDO on the last cycle if READOUT
                if readout and c == cycles then
                    f:write("#" .. (time + 75) .. "\n")
                    if ev.tdo and ev.tdo[1] ~= nil then
                        f:write((ev.tdo[1]==0 and "0" or "1") .. vcd_ids.TDO .. "\n")
                    else
                        f:write("x" .. vcd_ids.TDO .. "\n")
                    end
                else
                    f:write("x" .. vcd_ids.TDO .. "\n")
                end

                -- falling edge
                f:write("#" .. (time + 100) .. "\n0" .. vcd_ids.CLK .. "\n")
                time = time + 100
            end


        elseif ev.cmd == 0x03 then
            -- CMDXFER
            local cycles = ev.cycles or 1
            for c = 1, cycles do
                -- drive TDI at start of cycle
                f:write("#" .. (time +25) .. "\n")
                if ev.tdi and ev.tdi[c] ~= nil then
                    f:write((ev.tdi[c]==0 and "0" or "1") .. vcd_ids.TDI .. "\n")
                else
                    f:write("x" .. vcd_ids.TDI .. "\n")
                end

                -- rising edge
                f:write("#" .. (time + 50) .. "\n1" .. vcd_ids.CLK .. "\n")

                -- sample TDO
                f:write("#" .. (time + 75) .. "\n")
                if ev.tdo and ev.tdo[c] ~= nil then
                    f:write((ev.tdo[c]==0 and "0" or "1") .. vcd_ids.TDO .. "\n")
                else
                    f:write("x" .. vcd_ids.TDO .. "\n")
                end

                -- falling edge
                f:write("#" .. (time + 100) .. "\n0" .. vcd_ids.CLK .. "\n")
                time = time + 100
            end


        elseif ev.cmd == 0x05 then
            -- CMDGETSIG
            if ev.tdo ~= nil then
                
                f:write("#" .. (time - 25) .. "\n" ..(ev.tdo==0 and "0" or "1") .. vcd_ids.TDO .. "\n")
            else
                f:write("x" .. vcd_ids.TDO .. "\n")
            end
            f:write("#" .. time .. "\n")
        end
    end
    f:close()
    print("DirtyJTAG VCD log written to " .. filename)
end
-- Expose globals for console
_G.write_vcd = write_vcd
_G.reset_events = function()
    events = {}
    pending_out = {}
    print("DirtyJTAG events reset")
end
-- Decode SETSIG mask/values into human-readable signals
local function decode_setsig(cmd_item, mask, values)
    local function add_signal(name, bitmask)
        local masked = bit.band(mask, bitmask) ~= 0
        if masked then
            local val = bit.band(values, bitmask) ~= 0 and 1 or 0
            cmd_item:append_text(string.format(" %s=%d", name, val))
        else
            cmd_item:append_text(string.format(" %s=unchanged", name))
        end
    end

    add_signal("TCK",  0x02)
    add_signal("TDI",  0x04)  -- bit2
    add_signal("TMS",  0x10)
--    add_signal("TDO",  0x08)
    add_signal("TRST", 0x20)
    add_signal("SRST", 0x40)
end


-- Decode payload into bitstream
local function decode_bitstream(payload, bit_len)
    local bits = {}
    for i=0, payload:len()-1 do
        local b = payload(i,1):uint()
        for j=7,0,-1 do
            table.insert(bits, bit.band(b, 1 << j) ~= 0 and 1 or 0)
        end
    end
    -- Trim to requested length
    while #bits > bit_len do table.remove(bits) end
    return bits
end
-- === Dissector ===
function dirtyjtag.dissector(buffer, pinfo, tree)
    if buffer:len() < 1 then
        return false -- too short
    end

    local subtree = tree:add(dirtyjtag, buffer(), "DirtyJTAG Payload")
    pinfo.cols.protocol = "DirtyJTAG"

    local endpoint = pinfo.dst_port or 0
    local is_in  = bit.band(endpoint, 0x80) ~= 0
    local is_out = not is_in
    if is_out then
        local offset = 0
        local seqno = 0
        while offset < buffer:len() do
            seqno = seqno + 1
            if buffer:len() < offset+1 then return false end
            local cmd_val  = buffer(offset,1):uint()
            local base_cmd = bit.band(cmd_val, 0x0F)
            local cmd_name = cmd_names[base_cmd]
            if not cmd_name then return false end
            local ev = {dir="OUT", cmd=base_cmd, txn=seqno, seq = pinfo.number}
            local cmd_item = subtree:add(f_command, buffer(offset,1))
            cmd_item:append_text(" #" .. seqno .. " (" .. cmd_name .. ")")
            offset = offset + 1
            if base_cmd == 0x6 then -- CMD_CLK
                if buffer:len() >= offset+2 then
                    local sig_byte = buffer(offset,1):uint()
                    local cycles   = buffer(offset+1,1):uint()

                    -- Legal signals for CMD_CLK
                    local tms = bit.band(sig_byte, 0x10) ~= 0
                    local tdi = bit.band(sig_byte, 0x04) ~= 0

                    -- READOUT modifier (bit 0x80)
                    local readout = bit.band(cmd_val, 0x80) ~= 0
                    cmd_item:append_text(string.format(" TMS=%d TDI=%d%s",
                            tms and 1 or 0, tdi and 1 or 0,
                            readout and " [READOUT]" or "") .. " cycles=" ..cycles)

                    cmd_item:add(f_signal, buffer(offset,2))
                        :append_text(string.format(" TMS=%d TDI=%d%s",
                            tms and 1 or 0, tdi and 1 or 0,
                            readout and " [READOUT]" or "").. " cycles=" ..cycles)
                    offset = offset + 2

                    table.insert(pending_out, {
                        dir="OUT", cmd=base_cmd,
                        txn=seqno, seq = pinfo.number,
                        tms=tms, tdi=tdi,
                        cycles=cycles, readout=readout
                    })
                end

            elseif base_cmd == 0x4 then -- SETSIG
                if buffer:len() < offset+2 then return false end
                local mask   = buffer(offset,1):uint()
                local values = buffer(offset+1,1):uint()
                cmd_item:add(f_payload, buffer(offset,2))
                decode_setsig(cmd_item, mask, values)
                offset = offset + 2
                table.insert(pending_out, {dir="OUT", cmd=base_cmd, txn=seqno, seq = pinfo.number, mask=mask, values=values})

            elseif base_cmd == 0x3 then -- CMD_XFER
                if buffer:len() < offset+1 then return false end
                local raw_len = buffer(offset,1):uint()
                offset = offset + 1
                local bit_len = raw_len
                if bit.band(cmd_val, 0x40) ~= 0 then bit_len = bit_len + 256 end
                local byte_len = math.ceil(bit_len/8)
                local noread   = bit.band(cmd_val, 0x80) ~= 0

                cmd_item:append_text(string.format(
                    " length=%d bits (%d bytes)%s",
                    bit_len, byte_len, noread and " [NOREAD]" or ""))

                if buffer:len() >= offset+byte_len then
                    local payload = buffer(offset, byte_len)
                    cmd_item:add(f_payload, payload):append_text(" (XFER data)")

                    -- Decode payload into TDI bitstream
                    local tdi_bits = {}
                    for i=0, byte_len-1 do
                        local b = payload(i,1):uint()
                        for j=7,0,-1 do
                            table.insert(tdi_bits, bit.band(b, 1<<j) ~= 0 and 1 or 0)
                        end
                    end
                    if #tdi_bits > bit_len then
                        tdi_bits = {table.unpack(tdi_bits, 1, bit_len)}
                    end
                    local preview = {}
                    for i=1, math.min(bit_len,32) do
                        preview[#preview+1] = tdi_bits[i]
                        if i % 4 == 0 then preview[#preview+1] = " " end
                    end
                    cmd_item:append_text(" TDI="..table.concat(preview)..(bit_len>32 and "..." or ""))
                    offset = offset + byte_len

                    table.insert(pending_out, {
                        dir="OUT", cmd=base_cmd, 
                        txn=seqno, seq = pinfo.number,
                        cycles=bit_len,
                        noread=noread, tdi=tdi_bits
                    })
                else
                    return false
                end
            elseif base_cmd == 0x5 then -- GETSIG
                table.insert(pending_out, ev)
            elseif base_cmd < 0x2 then -- STOP and INFO
                offset = offset
            elseif base_cmd == 0x2 then -- FREQ
                if buffer:len() < offset+2 then return false end
                local freq = buffer(offset,2):le_uint()
                cmd_item:append_text(" Frequency=" .. freq .. " KHz")
                offset = offset + 2

            elseif base_cmd < 0x2 then -- STOP, INFO
                -- no payload

            else
                return false -- any other command not recognized
            end
        end

    elseif is_in then
        if buffer:len() == 0 then return false end
        local ev_in = {dir="IN"}
        -- Decode IN payload into TDO bitstream
        local in_tree = subtree:add(dirtyjtag, buffer(), "DirtyJTAG IN Payload")
        local byte_len = buffer:len()
        local bit_len  = byte_len * 8
        in_tree:append_text(string.format(" (%d bits, %d bytes)", bit_len, byte_len))
        in_tree:add(f_payload, buffer()):append_text(" (IN data)")

        local tdo_bits = decode_bitstream(buffer, buffer:len()*8)
        ev_in.tdo = tdo_bits
        if pinfo.visited then
            pending_out = {}
        else
            merge_in(ev_in)
        end
        -- processed_frames[pinfo.number] = true
    end


    return true
end

dirtyjtag:register_heuristic("usb.bulk", dirtyjtag.dissector)

-- global set


-- reset when a new capture file is opened
local function reset_processed_frames()
    -- processed_frames = {}
    events = {}
end

-- register a listener that fires on new capture
local cap_listener = Listener.new(nil, "frame")

function cap_listener.reset()
    reset_processed_frames()
end