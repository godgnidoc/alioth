use super::typing::SymbolID;
use std::collections::HashSet;

#[derive(Clone)]
pub struct MonoRegexNode {
    sub: Box<Regex>,
}

#[derive(Clone)]
pub struct BinaryRegexNode {
    left: Box<Regex>,
    right: Box<Regex>,
}

#[derive(Clone)]
pub enum LeafRegex {
    Accept(SymbolID),
    Char(char),
    Range {
        chars: HashSet<char>,
        includes: bool,
    },
}

#[derive(Clone)]
pub enum MonoRegex {
    Kleene(MonoRegexNode),
    Positive(MonoRegexNode),
    Optional(MonoRegexNode),
}

#[derive(Clone)]
pub enum BinaryRegex {
    Concat(BinaryRegexNode),
    Union(BinaryRegexNode),
}

#[derive(Clone)]
pub enum Regex {
    Leaf(LeafRegex),
    Mono(MonoRegex),
    Binary(BinaryRegex),
}

enum Unit {
    Char(char),
    Node(Regex),
}

fn extract_escape(c: char) -> Option<char> {
    Some(match c {
        'n' => '\n',
        'r' => '\r',
        't' => '\t',
        '\\' => '\\',
        '.' => '.',
        '\'' => '\'',
        '\"' => '"',
        '?' => '?',
        '*' => '*',
        '+' => '+',
        '|' => '|',
        '(' => '(',
        ')' => ')',
        '[' => '[',
        ']' => ']',
        '/' => '/',
        _ => return None,
    })
}

fn extract_class(c: char) -> Option<LeafRegex> {
    Some(match c {
        'd' => LeafRegex::Range {
            chars: ('0'..='9').collect(),
            includes: true,
        },
        'D' => LeafRegex::Range {
            chars: ('0'..='9').collect(),
            includes: false,
        },
        'l' => LeafRegex::Range {
            chars: ('a'..='z').collect(),
            includes: true,
        },
        'L' => LeafRegex::Range {
            chars: ('a'..='z').collect(),
            includes: false,
        },
        'u' => LeafRegex::Range {
            chars: ('A'..='Z').collect(),
            includes: true,
        },
        'U' => LeafRegex::Range {
            chars: ('A'..='Z').collect(),
            includes: false,
        },
        'w' => LeafRegex::Range {
            chars: ('a'..='z')
                .chain('A'..='Z')
                .chain('0'..='9')
                .chain(std::iter::once('_'))
                .collect(),
            includes: true,
        },
        'W' => LeafRegex::Range {
            chars: ('a'..='z')
                .chain('A'..='Z')
                .chain('0'..='9')
                .chain(std::iter::once('_'))
                .collect(),
            includes: false,
        },
        's' => LeafRegex::Range {
            chars: [' ', '\t', '\n', '\r'].into_iter().collect(),
            includes: true,
        },
        'S' => LeafRegex::Range {
            chars: [' ', '\t', '\n', '\r'].into_iter().collect(),
            includes: false,
        },
        'p' => LeafRegex::Range {
            chars: (0u8..=255)
                .filter(|&c| c.is_ascii_punctuation())
                .map(|c| c as char)
                .collect(),
            includes: true,
        },
        'P' => LeafRegex::Range {
            chars: (0u8..=255)
                .filter(|&c| c.is_ascii_punctuation())
                .map(|c| c as char)
                .collect(),
            includes: false,
        },
        _ => return None,
    })
}

fn parse_range(it: &mut std::str::Chars) -> Option<Regex> {
    let mut chars = HashSet::new();
    let mut includes = true;
    let mut lower_bound = None;

    let mut state = 1;

    while state > 0 {
        if let Some(c) = it.next() {
            match state {
                1 => match c {
                    '^' => {
                        includes = false;
                        state = 2;
                    }
                    ']' => {
                        state = 0;
                    }
                    '\\' => {
                        if let Some(cc) = it.next() {
                            if let Some(esc) = extract_escape(cc) {
                                chars.insert(esc);
                            } else if let Some(class) = extract_class(cc) {
                                match class {
                                    LeafRegex::Range { chars: c, includes: _ } => {
                                        chars.extend(c);
                                    }
                                    _ => return None,
                                }
                            } else {
                                return None;
                            }
                        } else {
                            return None;
                        }
                    }
                    _ => {
                        lower_bound = Some(c);
                        chars.insert(c);
                        state = 2;
                    }
                },
                2 => match c {

                }
                3 => {}
                4 => {}
            }
        } else {
            return None;
        }
    }

    return Some(Regex::Leaf(range));
}

fn tokenize(input: &str) -> Option<Vec<Unit>> {
    let mut units = Vec::new();
    let mut it = input.chars();
    while let Some(c) = it.next() {
        match c {
            '[' => {
                let range = parse_range(&mut it)?;
                units.push(Unit::Node(range));
            }
            '\\' => {
                if let Some(cc) = it.next() {
                    if let Some(esc) = extract_escape(cc) {
                        units.push(Unit::Char(esc));
                    } else if let Some(class) = extract_class(cc) {
                        units.push(Unit::Node(Regex::Leaf(class)));
                    } else {
                        return None;
                    }
                } else {
                    return None;
                }
            }
            '.' => {
                let any = LeafRegex::Range {
                    chars: HashSet::new(),
                    includes: false,
                };
                units.push(Unit::Node(Regex::Leaf(any)));
            }
            _ => {
                units.push(Unit::Char(c));
            }
        }
    }

    Some(units)
}

fn parse(units: &mut Vec<Unit>, pos: usize) {}

pub fn compile(pattern: &str) -> Option<Regex> {
    let mut units = tokenize(pattern)?;
    parse(&mut units, 0);

    if units.iter().count() != 1 {
        return None;
    }

    if let Unit::Node(regex) = &units[0] {
        Some(regex.clone())
    } else {
        None
    }
}
