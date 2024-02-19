if [ "$#" -lt 0 ]; then
  echo "Usage: $0 [CLANG_ARGS]" >&2
  exit 1
fi

mkdir -p ./out

clang recursive_pcall.c -o ./out/recursive_pcall $1 > /dev/null
