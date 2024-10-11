;;; secrec-mode.el --- An Emacs mode for SecreC source code

(require 'generic-x)

(define-generic-mode
    'secrec-mode

  ;; comment-list
  '("//" ("/*" . "*/"))

  ;; keyword-list, every keyword highlighted with `font-lock-keyword-face`
  '(
    ;; keywords, list from ./doc/secrec/semantics.tex::106
    "bool" "break"
    "continue" "declassify" "do" "else"
    "false" "for" "if" "int" "private"
    "public" "return" "string" "true"
    "unsigned" "void" "while" "size"
    "shape" "reshape" "cat"
    ;; data types
    "void" "string" "bool"
    "uint" "uint8" "uint16" "uint32" "uint64"
    "int" "int8" "int16" "int32" "int64"
    "float32" "float64"

    "import" "domain" "dim" "template" "type" "D" "struct"
    )

  ;; font-lock-list : list of additional expressions to highlight
  '(("=" . 'font-lock-operator)
    (";" . 'font-lock-builtin))

  ;; auto-mode-list
  '("\\.sc$")

  ;; function-list : functions for additional setup, called before mode-hook
  nil

  ;; &optional docstring
  "An Emacs mode for SecreC source code"
  )
