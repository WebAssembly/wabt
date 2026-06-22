#!/usr/bin/env lua

local format_br = '\n.br\n'
local tools = {
	"wasm-decompile",	"wasm-interp",	"wasm-objdump",	"wasm-stats",
	"wasm-strip",	"wasm-validate",	"wasm2c",	"wasm2wat",
	"wast2json",	"wat-desugar",	"wat2wasm",	"spectest-interp"
}

local descriptions = {
	["spectest-interp"] = [[read a Spectest JSON file, and run its tests in the interpreter]],
	["wasm-interp"] = [[decode and run a WebAssembly binary file]],
	["wasm-objdump"] = [[print information about a wasm binary]],
	["wasm-stats"] = [[show stats for a module]],
	["wasm-strip"] = [[remove sections of a WebAssembly binary file]],
	["wasm-validate"] = [[validate a file in the WebAssembly binary format]],
	["wasm2c"] = [[convert a WebAssembly binary file to a C source and header]],
	["wasm2wat"] = [[translate from the binary format to the text format]],
	["wast2json"] = [[convert a file in the wasm spec test format to a JSON file and associated wasm binary files]],
	["wat-desugar"] = [[parse .wat text form and print "canonical" flat format]],
	["wat2wasm"] = [[translate from WebAssembly text format to the WebAssembly binary format]],
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
for index, tool in ipairs(tools) do
	if tool == name then
		table.remove(tools, index)
		break
	end
end



do -- print the man page
	print(".Dd $Mdocdate$")
	print(".Dt WABT 1")
	print(".Os")
	print(".Sh NAME")
	print((".Nm %s"):format(name))
	local desc = (short and short:match("%S")) and short or descriptions[name] or ""
	print((".Nd %s"):format(desc))
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
	for idx, tool in ipairs(tools) do
		print((".Xr %s 1 %s"):format(tool, idx == #tools and "" or ","))
	end
	print(".Sh BUGS")
	print("If you find a bug, please report it at")
	print(".br")
	print(".Lk https://github.com/WebAssembly/wabt/issues .")
end
