import * as vscode from "vscode";
import { activate as activateTemplate } from './template'
import { activate as activateGrammar } from './grammar'

export function activate(context: vscode.ExtensionContext) {
    console.log("Alioth extension activated");
    activateTemplate(context);
    activateGrammar(context);
}

export function deactivate() {
}