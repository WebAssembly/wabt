#!/usr/bin/env lua

local format_br = '\n.br\n'
local tools = {
	"wasm-decompile",	"wasm-interp",	"wasm-objdump",	"wasm-opcodecnt",
	"wasm-strip",	"wasm-validate",	"wasm2c",	"wasm2wat",
	"wast2json",	"wat-desugar",	"wat2wasm",	"spectest-interp"
}

local name, mainarg, argvs
local usage
local short = ""
local full = {}
local examples = {}
local options = {}

local last = { }

do -- parse usage
	local usage = io.read('*line');
	name, mainarg, argvs = usage:match("usage: ([^%s%c]+) %[options%] ([^%s%c]+) %[([^%s%c]+)%]%.%.%.")
	if not argvs then
		name, mainarg = usage:match("usage: ([^%s%c]+) %[options%] ([^%s%c]+)")
	end
end

do -- read short description
	repeat line = io.read('*line');
		short = short .. ' ' .. line
	until line == ""
end

do -- read full description
	repeat line = io.read('*line');
		table.insert(full, line)
	until line == "examples:"
	do -- format full description
		table.remove(full, #full) -- remove "examples:"
		full = table.concat(full, "") -- concat everything with a space
		full = full:gsub("^  ", "")
		full = full:gsub("\n  ", "")
		-- full = full:gsub("  +", "\n")
		full:gsub('\n', format_br)
	end
end


do -- parse examples
	line = io.read('*line'); -- skip "examples:"
	last = { descr = {} } -- prepare new example
	while line ~= "options:" do
		-- empty lines are between examples
		if line:match("^%s*$") then
			-- format last
			local descr = last.descr
			descr = table.concat(descr, '\n')
			last.descr = descr
			-- push last
			table.insert(examples, last)
			-- prepare new example
			last = { descr = {} }
		end
		
		-- found the sample usage
		if line:match("^%s*%$ ") then
			last.input = line:match("^ *%$ (.+)")
		end
		-- found some description
		if line:match("^ *# ") then
			table.insert(last.descr, (line:gsub("^ *# ", "")))
		end
		
		line = io.read('*line');
	end
end


do -- parse options
	last = { }
	for line in io.stdin:lines() do
		if line:match("^ +%-") then
			table.insert(options, last)
			
			local short, long, descr = line:match("^  %-(%w), %-%-([^%s%c]+) +(.+)")
			if not short then
				long, descr = line:match("^      %-%-([^%s%c]+) +(.+)")
			end
			last = {
				short = short,
				long = long,
				descr = descr,
			}
		else
			last.descr = last.descr .. line:gsub("^ +", "\n")
		end
	end
	table.remove(options, 1)
	last = { }
end


-- remove this tool from the SEE ALSO section
for index, tool in pairs(tools) do
	if tool == name then
		table.remove(tools, index)
		break
	end
end



do -- print the man page
	print(".Dd $Mdocdate$")
	print(".Os")
	print(".Sh NAME")
	print((".Nm %s"):format(name))
	print((".Nd %s"):format(short))
	print(".Sh SYNOPSIS")
	print((".Nm %s"):format(name))
	print(".Op options")
	print((".Ar %s"):format(mainarg))
	if argvs then
		print((".Ar [%s]..."):format(argvs))
	end
	print(".Sh DESCRIPTION")
	print(".Nm")
	print(full)
	print(".Pp")
	print("The options are as follows:")
	print(".Bl -tag -width Ds")
	for index, option in pairs(options) do
		if option.short then
			print((".It Fl %s , Fl Fl %s"):format(option.short, option.long))
		else
			print((".It Fl Fl %s"):format(option.long))
		end
		print(option.descr)
	end
	print(".El")
	print(".Sh EXAMPLES")
	for index, example in pairs(examples) do
		print(example.descr)
		print(".Pp")
		print((".Dl $ %s"):format(example.input))
	end
	print(".Sh SEE ALSO")
	for idx, tool in pairs(tools) do
		print((".Xr %s 1 %s"):format(tool, idx == #tool and "" or ","))
	end
	print(".Sh BUGS")
	print("If you find a bug, please report it at")
	print(".br")
	print(".Lk https://github.com/WebAssembly/wabt/issues .")
end
