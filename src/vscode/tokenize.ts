
import * as vscode from 'vscode';
import { spawnSync } from "child_process";
import { join } from "path";

interface Point {
    line: number;
    column: number;
}

interface Range {
    start: Point;
    end: Point;
}

interface Token {
    tokenize?: {
        type?: string,
        modifier?: string[]
    }
    range: Range;
}

export function tokenizer(context: vscode.ExtensionContext) {
    return function tokenizeDocument(document: vscode.TextDocument, grammar: string): vscode.SemanticTokens {
        console.log("Tokenizing document: " + document.fileName);
        const aliothHome = context.extensionPath;

        const proc = spawnSync(join(aliothHome, "build/x64-linux-release/alioth"), [
            "parse",
            "-u",
            "-f",
            "-C",
            "--range",
            "range",
            "-",
            "--grammar",
            join(aliothHome, `grammar/${grammar}.grammar`),
        ], {
            shell: '/bin/bash',
            input: document.getText(),
            encoding: 'utf-8'
        });

        const tokens: Token[] = JSON.parse(proc.output[1]!)
        const tokensBuilder = new vscode.SemanticTokensBuilder(standardLegend);
        const lines = document.getText().split(/\r?\n/);
        for (const token of tokens) {
            if (!token.tokenize?.type) continue

            for (let l = token.range.start.line; l <= token.range.end.line; l++) {
                const line = l - 1
                const start_column = (l === token.range.start.line
                    ? token.range.start.column - 1
                    : 0)
                const end_column = (l === token.range.end.line
                    ? token.range.end.column - 1
                    : lines[line].length)

                const range = new vscode.Range(new vscode.Position(
                    line,
                    start_column,
                ), new vscode.Position(
                    line,
                    end_column,
                ))

                tokensBuilder.push(
                    range,
                    token.tokenize.type,
                    token.tokenize.modifier
                )
            }
        }
        return tokensBuilder.build();
    }
}



const standardTypes = [
    "namespace",
    "class",
    "enum",
    "interface",
    "struct",
    "typeParameter",
    "type",
    "parameter",
    "variable",
    "property",
    "enumMember",
    "decorator",
    "event",
    "function",
    "method",
    "macro",
    "label",
    "comment",
    "string",
    "keyword",
    "number",
    "regexp",
    "operator",
]

const standardModifiers = [
    "declaration",
    "definition",
    "readonly",
    "static",
    "deprecated",
    "abstract",
    "async",
    "modification",
    "documentation",
    "defaultLibrary",
]

export const standardLegend = new vscode.SemanticTokensLegend(
    standardTypes,
    standardModifiers
);