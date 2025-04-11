import * as vscode from "vscode";
import { spawnSync } from "child_process";
import { workspace, languages } from "vscode";
import { join } from "path";

const types = ["variable", "comment", "string", "number", "keyword", "operator"]
const modifiers: string[] = []

const legend = new vscode.SemanticTokensLegend(types, modifiers);

interface Point {
    line: number;
    column: number;
}

interface Range {
    start: Point;
    end: Point;
}

interface Token {
    id: number;
    name: string;
    range: Range;
}

const types_mapping: { [name: string]: string } = {
    "TXT": "string",
    "COMMENT": "comment",
    "LOPEN": "operator",
    "ROPEN": "operator",
    "LCLOSE": "operator",
    "RCLOSE": "operator",
    "RCOMMENT": "operator",
    "DASH": "operator",
    "DOUBLEDASH": "comment",
    "AT": "operator",
    "OPEN_SUB": "operator",
    "CLOSE_SUB": "operator",
    "OPEN_IDX": "operator",
    "CLOSE_IDX": "operator",
    "PIPE": "operator",
    "COMMA": "operator",
    "DOT": "operator",
    "EXTENDS": "keyword",
    "OVERWRITE": "keyword",
    "BLOCK": "keyword",
    "CALL": "keyword",
    "WITH": "keyword",
    "FOR": "keyword",
    "IN": "keyword",
    "IF": "keyword",
    "ELSE": "keyword",
    "THEN": "keyword",
    "END": "keyword",
    "STRING": "string",
    "NUMBER": "number",
    "ID": "variable",
}

const provider: vscode.DocumentSemanticTokensProvider = {
    provideDocumentSemanticTokens(
        document: vscode.TextDocument
    ): vscode.ProviderResult<vscode.SemanticTokens> {
        console.log("Tokenizing document: " + document.fileName);

        // analyze the document and return semantic tokens
        const aliothHome = "/home/godgnidoc/projects/alioth/";
        const command = join(aliothHome, "build/x64-linux-debug/alioth")
            + " tokenize"
            + " -"
            + " --grammar"
            + " /home/godgnidoc/projects/alioth/grammar/template.grammar"

        const proc = spawnSync(command, {
            shell: '/bin/bash',
            input: document.getText(),
            encoding: 'utf-8'
        });

        const tokens: Token[] = JSON.parse(proc.output[1]!)
        const tokensBuilder = new vscode.SemanticTokensBuilder(legend);
        const lines = document.getText().split(/\r?\n/);
        for (const token of tokens) {
            if (token.name === "SPACE") continue
            if (token.name === "<ERR>") continue

            // 临时跳过多行单词
            // if (token.range.start.line !== token.range.end.line) continue
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
                    types_mapping[token.name],
                    []
                )
            }
        }
        return tokensBuilder.build();
    }
};

const selector = { language: 'template', scheme: 'file' }; // register for all Java documents from the local file system


export function activate(context: vscode.ExtensionContext) {
    console.log("Alioth extension activated");
    vscode.languages.registerDocumentSemanticTokensProvider(selector, provider, legend);
}

export function deactivate() {
}