import json
from os import path


def main():
    with open('./compile_commands.json') as inf:
        data = json.loads(inf.read())
    for compilation in data:
        p = path.abspath(path.join(compilation['directory'], compilation['file']))
        print(p)


if __name__ == '__main__':
    main()
