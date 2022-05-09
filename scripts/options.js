const { readFileSync, writeFileSync } = require('fs');
const { argv } = require('process');

let source = readFileSync(argv[2]).toString();

source = source.match(/enum\s*global_options\s*{((?:.|\n)+)};/m)[1]

let state = 1;
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
            if (line == "*/") {
                state = 4
            } else if (line.startsWith("* @default ")) {
                global_options[global_options.length - 1].required = false
                global_options[global_options.length - 1].brief += "default: " + line.slice("* @default ".length)
            } else if (line.startsWith("* @brief ")) {
                global_options[global_options.length - 1].brief += line.slice("* @brief ".length)
            } else {
                global_options[global_options.length - 1].brief += "\n" + line.slice("* ".length)
            }
        } break
        case 4: {
            global_options[global_options.length - 1].ID = line.match(/((?:\w|_)+)/)[1]
            state = 1
        } break
    }
}

let code = ''

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

source = readFileSync(argv[3]).toString()

source = source.replace(
    /\/\*\* begin-code-gen-mark:global-options \*\/(.|\n)+\/\*\* end-code-gen-mark:global-options \*\//m,
    `/** begin-code-gen-mark:global-options */${code}/** end-code-gen-mark:global-options */`
)

writeFileSync(argv[3], source)
// console.log(source)