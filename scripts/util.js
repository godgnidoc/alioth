
/**
 * @param {string} mark
 * @param {string} source
 * @param {string} fragment
 */
function fill_marked_fragment(mark, source, fragment) {
    let match = new RegExp(`\\/\\*\\* begin-code-gen-mark:${mark} \\*\\/(.|\\n)+\\/\\*\\* end-code-gen-mark:${mark} \\*\\/`, 'm')
    fragment = `/** begin-code-gen-mark:${mark} */${fragment}/** end-code-gen-mark:${mark} */`
    let old = source.match(match)
    if (old)
        return source.replace(old[0], fragment)
    return source
}

module.exports = {
    fill_marked_fragment
}