let gcc_suffix = '-14'
let clang_suffix = '-19'

let g:ale_c_clangformat_executable = 'clang-format' . clang_suffix

let g:ale_c_cc_executable = 'gcc' . gcc_suffix
let g:ale_cpp_cc_executable = 'g++' . gcc_suffix
