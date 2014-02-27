" Vim indent file
" Language:	SecreC
" Identical to the C indent file by Bram Molenaar.

" Only load this indent file when no other was loaded.
if exists("b:did_indent")
   finish
endif
let b:did_indent = 1

" Use the builtin C indenting
setlocal cindent

let b:undo_indent = "setl cin<"
