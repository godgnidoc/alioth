import * as vscode from 'vscode';
import { standardLegend, tokenizer } from './tokenize';

const selector = { language: 'template', scheme: 'file' };

export function activate(context: vscode.ExtensionContext) {
    const grammar = "template"
    const tokenize = tokenizer(context)

    const provider: vscode.DocumentSemanticTokensProvider = {
        provideDocumentSemanticTokens(
            document: vscode.TextDocument
        ): vscode.ProviderResult<vscode.SemanticTokens> {
            return tokenize(document, grammar)
        }
    };
    vscode.languages.registerDocumentSemanticTokensProvider(
        selector, provider, standardLegend);
}