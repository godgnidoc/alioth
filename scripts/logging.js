const { readFileSync, writeFileSync } = require('fs');
const { join } = require('path');
const { fill_marked_fragment } = require('./util')

const logging_json = join(__dirname, '..', 'doc', 'logging.json')
const logger_cpp = join(__dirname, '..', 'src', 'logger.cpp')
const logger_helper_hpp = join(__dirname, '..', 'inc', 'logger.helper.hpp')
const logger_helper_cpp = join(__dirname, '..', 'src', 'logger.helper.cpp')
const templates = JSON.parse(readFileSync(logging_json).toString())["logging"]["templates"];

let code = '\n        '
let defs = ''
let impls = ''
let eno = 0
for (let ename in templates) {

    let tmpl = templates[ename]
    let match = tmpl.match(/\$\(\w+\)/g)
    let params = ''
    let args = ''
    for( let param of match ) {
        let pname = param.slice(2,param.length-1)
        params += `, const std::string& ${pname}`
        args += `{"${pname}",${pname}},`
    }
    defs += `\n    /** ${tmpl} */\n    void ${ename}(severity sev, range rng${params});\n    `;
    defs += `/** ${tmpl} */\n    void ${ename}(severity sev${params});\n    `;

    impls += `\nvoid helper::${ename}(severity sev, range rng${params}) { l.emit(sev, rng, ${eno}, {${args}}); }\n`
    impls += `void helper::${ename}(severity sev${params}) { l.emit(sev, range(), ${eno}, {${args}}); }\n`

    code += `{"${ename}", ${eno++}},\n        `
}

let source = readFileSync(logger_cpp).toString()
source = fill_marked_fragment('logging-error-code', source, code)
writeFileSync(logger_cpp, source)

source = readFileSync(logger_helper_hpp).toString()
source = fill_marked_fragment('logging-helper-def', source, defs)
writeFileSync(logger_helper_hpp, source)

source = readFileSync(logger_helper_cpp).toString()
source = fill_marked_fragment('logging-helper-impl', source, impls)
writeFileSync(logger_helper_cpp, source)
// console.log(source)