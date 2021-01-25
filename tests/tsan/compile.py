#!/usr/bin/python
import sys
import os
import argparse


FOLDER = os.path.dirname(__file__)

DEFAULT_INCLUDE_FOLDERS = [
    '${LLVM_ROOT}/compiler-rt/include/',
    '${LLVM_ROOT}/compiler-rt/test/',
    '${LLVM_ROOT}/compiler-rt/test/tsan/',
]

DEFAULT_FEATURES = [
    'clwb',
]


def compile_with_tsan(source_files, include_folders, out_path, features, verbose=False, generate_debug_info=True):
    cmd = 'clang {} -fsanitize=thread -fPIE -pie {} {} {} -o {} {}'.format(
        ' '.join(f'"{f}"' for f in source_files),
        '-g' if generate_debug_info else '',
        ' '.join(f'-I "{i}"' for i in include_folders),
        ' '.join(f'-m{f}' for f in features),
        out_path,
        '-v' if verbose else '',
    )
    os.system('echo ' + cmd)
    return os.system(cmd)


def expand_files(folders_and_files):
    files = []
    for f in folders_and_files:
        if os.path.isfile(f):
            files.append(f)
        elif os.path.isdir(f):
            folders_and_files.extend([os.path.join(f, d) for d in os.listdir(f)])
        else:
            raise Exception(f'File or folder {f} not found')

    return files


def parse_args():
    p = argparse.ArgumentParser()
    
    p.add_argument('-s', '--source', nargs='+', default=[os.path.join(FOLDER, 'src')], help='Source files and folders')
    p.add_argument('-i', '--include', nargs='+', default=DEFAULT_INCLUDE_FOLDERS, help='Include folders')
    p.add_argument('-f', '--features', nargs='+', default=DEFAULT_FEATURES, help='Add compiler features')
    p.add_argument('-o', '--out', default=os.path.join(FOLDER, 'out.a'), help='Output file path')
    p.add_argument('-r', '--run', action='store_true',  help='Run after compiling')
    p.add_argument('-d', '--gdb', action='store_true', help='Run with gdb')
    p.add_argument('-v', '--verbose', action='store_true', help='Verbose compiler')

    return p.parse_args()


def main():
    args = parse_args()

    source_files = expand_files(args.source)

    if 0 != compile_with_tsan(source_files, args.include, args.out, args.features, args.verbose):
        return

    if args.gdb:
        print(f'Running {args.out} with gdb')
        os.system(f'gdb {args.out}')
    elif args.run:
        print(f'Running {args.out}')
        os.system(args.out)
        

if __name__ == "__main__":
    main()
