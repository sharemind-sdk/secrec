" Vim syntax file
" Language:	SecreC
" Hacked together from the C vim syntax file by Bram Moolenaar and
" secrec.tmLanguage for Sublime Text.

" Quit when a syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

syn keyword	scTypes		bool void string
syn keyword	scTypes		int int8 int16 int32 int64
syn keyword	scTypes		uint uint8 uint16 uint32 uint64
syn keyword	scTypes		xor_uint8 xor_uint16 xor_uint32 xor_uint64
syn keyword	scTypes		float float32 float64

syn keyword	scStatement	break return continue
syn keyword	scConditional	if else
syn keyword	scRepeat	while for do

" TODO: [A-Z]([0-9]{0,1}) ?

syn keyword	scModifier	template domain kind dim type module import public operator

syn keyword	scConstant	true false

syn keyword	scSupport	cat size shape reshape tostring assert declassify
syn keyword	scSupport	__domainid __syscall __ref __cref __return __bytes_from_string __string_from_bytes

syn match	scNumber	"\<\(0x\)\=\d\+\(\.\d\+\)\="
syn match	scNumber	"\<0b[01]\+"

syn match	scSpecial	contained "\\\(x\x\+\|\o\{1,3}\)"
syn match	scSpecial	contained "\\\(u\x\{4}\|U\x\{8}\)"
syn match	scSpecial	contained "\\\(['\"?\\abfnrtv]\|$\)"
syn region	scString	start=+"+ skip=+\\\\\|\\"+ end=+"+ contains=scSpecial extend

syn match	scCharacter	"'\\\(x\x{1,2}\|\o\{1,3}\)'" contains=scSpecial
syn match	scCharacter	"'\\['\"?\\abfnrtv]'" contains=scSpecial
syn match	scCharacter	"'[^\\]'"

syn match	scBlockError	"}"
syn region	scBlock		transparent start="{" end="}" contains=ALLBUT,scBlockError fold

syn match	scParenError	")"
syn region	scParen		transparent start='(' end=')' contains=ALLBUT,scParenError,scBlock

syn match	scBraceError	"\]"
syn region	scBrace		transparent start='\[' end='\]' contains=ALLBUT,scBraceError,scBlock

syn keyword	scTodo		contained TODO FIXME XXX
syn region	scCommentL	start="//" skip="\\$" end="$" contains=scTodo,scNumberCom keepend

syn match	scCommentError	"\*/"
syn region	scComment	start="/\*" end="\*/" contains=scTodo

" Define the default highlighting.
" Only used when an item doesn't have highlighting yet
hi def link scTypes		Type
hi def link scStatement		Statement
hi def link scConditional	Conditional
hi def link scRepeat		Repeat
hi def link scModifier		Statement
hi def link scConstant		Constant
hi def link scSupport		Operator
hi def link scNumber		Number
hi def link scSpecial		SpecialChar
hi def link scString		String
hi def link scCharacter		Character
hi def link scBlockError	Error
hi def link scParenError	Error
hi def link scBraceError	Error
hi def link scTodo		Todo
hi def link scCommentL		scComment
hi def link scCommentError	Error
hi def link scComment		Comment

let b:current_syntax = "secrec"

let &cpo = s:cpo_save
unlet s:cpo_save
" vim: ts=8
