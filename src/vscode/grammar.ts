import * as vscode from 'vscode';
import { tokenizer } from './tokenize';

const selector = { language: 'grammar', scheme: 'file' }; // register for all Java documents from the local file system
const types = ["regexp", "variable", "comment", "string", "number", "keyword", "operator"]
const modifiers: string[] = []
const legend = new vscode.SemanticTokensLegend(types, modifiers);
const types_mapping: { [name: string]: string } = {
    "LEAD": "operator",
    "LT": "operator",
    "GT": "operator",
    "UNION": "operator",
    "DEFINE": "operator",
    "IGNORE": "operator",
    "AT": "operator",
    "SEMICOLON": "operator",
    "COLON": "operator",
    "COMMA": "operator",
    "DOT": "operator",
    "UNFOLD": "operator",
    "LBRACE": "operator",
    "RBRACE": "operator",
    "LBRACKET": "operator",
    "RBRACKET": "operator",
    "EMPTY": "keyword",
    "NULL": "keyword",
    "TRUE": "keyword",
    "FALSE": "keyword",
    "STRING": "string",
    "NUMBER": "number",
    "ID": "variable",
    "REGEX": "regexp",
    "COMMENT": "comment",
}

export function activate(context: vscode.ExtensionContext) {
    const grammar = "grammar"
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