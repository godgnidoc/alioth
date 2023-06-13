#ifndef __ALIOTH_LEX_SCANNER_FWD_H__
#define __ALIOTH_LEX_SCANNER_FWD_H__

#include <memory>

namespace alioth::lex {

class Scanner;
using ScannerRef = std::shared_ptr<Scanner>;

}  // namespace alioth::lex

#endif