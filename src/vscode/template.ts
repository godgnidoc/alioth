import * as vscode from 'vscode';
import { tokenizer } from './tokenize';

const selector = { language: 'template', scheme: 'file' }; // register for all Java documents from the local file system
const types = ["variable", "comment", "string", "number", "keyword", "operator"]
const modifiers: string[] = []
const legend = new vscode.SemanticTokensLegend(types, modifiers);
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

export function activate(context: vscode.ExtensionContext) {
    const grammar = "template"
    const tokenize = tokenizer(context)

    const provider: vscode.DocumentSemanticTokensProvider = {
        provideDocumentSemanticTokens(
            document: vscode.TextDocument
        ): vscode.ProviderResult<vscode.SemanticTokens> {
            return tokenize(document, grammar, legend, types_mapping)
        }
    };
    vscode.languages.registerDocumentSemanticTokensProvider(selector, provider, legend);
}