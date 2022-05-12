const { readFile, writeFile } = require('fs/promises');
const { join } = require('path');
const {argv} = require('process');
const { promisify } = require('util')
const child = require('child_process')

const about_path = join(__dirname, '..', 'doc', 'about.json');

const exec = promisify(child.exec);

function dump_version(v) {
    console.log(`${v.major}.${v.minor}.${v.patch}-build${v.build}`)
}

(async function() {
    let about = JSON.parse(await (await readFile(about_path)).toString())
    
    switch( argv[2] ) {
        case 'name': {
            console.log(about.name)
        } break
        case 'build': {
            about.version.build += 1
            dump_version(about.version);
        } break
        case 'patch': {
            about.version.patch += 1
            about.version.build = 0
            dump_version(about.version);
        } break
        case 'minor': {
            about.version.minor += 1
            about.version.patch = 0
            about.version.build = 0
            dump_version(about.version);
        } break
        case 'major': {
            about.version.major += 1
            about.version.minor = 0
            about.version.patch = 0
            about.version.build = 0
            dump_version(about.version);
        } break
        case 'version': {
            dump_version(about.version)
        } break
        case 'commit': {
            console.log((await exec('git rev-parse HEAD')).stdout.trim())
        } break
        case 'brief': {
            console.log(about.brief)
        } break
        case 'license': {
            console.log(about.license)
        } break
        case 'publisher.name': {
            console.log(about.publisher.name)
        } break
        case 'publisher.email': {
            console.log(about.publisher.email)
        } break
        case 'arch': {
            console.log(process.arch)
        } break
        case 'os': {
            console.log(process.platform)
        } break
    }

    await writeFile(about_path, JSON.stringify(about, undefined, 4))
})()

