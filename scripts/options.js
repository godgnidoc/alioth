const { readFileSync, writeFileSync } = require('fs');
const { join } = require('path');
const { fill_marked_fragment } = require('./util');

/**
 * @script 此脚本用于从全局选项ID枚举及其注释生成全局选项注册代码
 */

const compiler_hpp = join(__dirname, '..', 'inc', 'compiler.hpp')
const compiler_options_hpp = join(__dirname, '..', 'src', 'compiler.options.cpp')

let source = readFileSync(compiler_hpp).toString();
source = source.match(/enum\s*global_options\s*{((?:.|\n)+)};/m)[1]

let state = 1
let global_options = []
for (let line of source.split('\n')) {
    line = line.trim()
    switch (state) {
        case 1: {
            if (line == "/**")
                state = 2
        } break
        case 2: {
            let res = line.match(/\*\s+((?:\w|-|_)+)\s*((?:\w|\/|-|_|<|>|\.|\s)*)?\s*(\.\.\.)?/)
            let name = res[1]
            let options = res[2]
            let times = res[3] ? 0 : 1
            let args = options ? options.split(/>\s+</).length : 0
            global_options.push({ name, args, required: true, brief: `usage: ${name} ${options}\n`, times, ID: '' })
            state = 3
        } break
        case 3: {
            let opt = global_options[global_options.length - 1]
            if (line == "*/") {
                state = 4
            } else if (line.startsWith("* @default ")) {
                opt.required = false
                let def = line.slice("* @default ".length)
                opt.brief += "default: ".padStart(41) + def + '\n'
            } else if (line.startsWith("* @brief ")) {
                opt.brief += " ".repeat(32) + line.slice("* @brief ".length) + "\n"
            } else {
                opt.brief += " ".repeat(32) + line.slice("* ".length) + "\n"
            }
        } break
        case 4: {
            let opt = global_options[global_options.length - 1]
            opt.ID = line.match(/((?:\w|_)+)/)[1]
            state = 1
        } break
    }
}

let code = ''
global_options = global_options.sort((a, b) => a.name > b.name ? 1 : -1)

for (let go of global_options) {
    code += `
    regist_global_option(${go.ID}, (cli::option){
        name : ${JSON.stringify(go.name)},
        brief : ${JSON.stringify(go.brief)},
        args : ${go.args},
        times : ${go.times},
        required : ${go.required}
    });
    `
}

source = readFileSync(compiler_options_hpp).toString()
source = fill_marked_fragment('global-options', source, code)

writeFileSync(compiler_options_hpp, source)
// console.log(source)