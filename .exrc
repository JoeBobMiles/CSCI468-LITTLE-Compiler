let g:neomake_c_enabled_makers = ['gcc']
let g:neomake_c_gcc_args = ['-fsyntax-only', '-Wall', '-Wextra', '-Wpedantic']

augroup ChangeCOptions
    au BufRead *.\(cpp\|h\) syn keyword cType s8 s16 s32 s64 u8 u16 u32 u64 r32 r64
    au BufRead *.\(cpp\|h\) syn keyword cType cchar
    au BufRead *.\(cpp\|h\) syn keyword cStatement fallthrough
    au BufRead *.\(cpp\|h\) syn keyword cStorageType macro
    au BufRead *.\(cpp\|h\) compiler gcc
augroup END

let g:compiler_gcc_ignore_unmatched_lines = 1
