import argparse
from enum import Enum

AST = []
STRUCT_SIZES = {}

class Lang(Enum):
    STRUCT_PACK = 0
    C = 1
    SIZE = 2

struct_symbols = {
    "bool"   : ("?", "_Bool", 1),

    "u_int8" : ("B", "unsigned char", 1),
    "u_int16": ("H", "unsigned short", 2),
    "u_int32": ("I", "unsigned long", 4),
    "u_int64": ("Q", "unsigned long long", 8),

    "int8" : ("b", "char", 1),
    "int16": ("h", "short", 2),
    "int32": ("i", "long", 4),
    "int64": ("q", "long long", 8),

    "float" : ("f", "float", 4),
    "double": ("d", "double", 8)
}


def parse_input(input_file: str):
    with open(input_file, "r") as fl:
        r = fl.read()
        obj = {}
        for x in r.split("\n"):
            if len(x) > 0:
                unit = x.strip().split()
                if unit[0] == "struct":
                    if obj:
                        AST.append(obj)
                    obj = {"obj":unit[1][:-1]}
                else:
                    obj.update({unit[0]:unit[1]})
        AST.append(obj)

def type_check(ctype, obj):
    if ctype not in struct_symbols and ctype not in STRUCT_SIZES:
        raise ValueError(f"Unknown type '{ctype}' in struct '{obj['obj']}'")

def generate_python_classes():
    res = "import struct\n\n"
    for obj in AST:
        items = list(obj.items())[1:]

        # class declaration and params init ----------------------------------------- 

        res += f"class {obj['obj']}:\n"
        size = sum(
            struct_symbols[x[1]][Lang.SIZE.value]
            if x[1] in struct_symbols
            else STRUCT_SIZES[x[1]]
            for x in items
        )

        STRUCT_SIZES.update({obj['obj'] : size})

        res += f"    size = {size}\n\n"
        res += f"    def __init__(self, "
        for param, _ in items:
            res += f"{param}=None, "
        res += "):\n"


        for param, _ in items:
            res += f"        self.{param} = {param}\n"
        res += "\n"
        
        # from_binary ---------------------------------------------------------------

        res += "    def from_binary(self, data):\n"
        res += "        if len(data) < self.size:\n"
        res += "           raise ValueError(f'Insufficient data for {self.__class__.__name__}') \n"
        count = 0
        for param, ctype in items:
            type_check(ctype, obj)
            if ctype not in struct_symbols.keys():
                res += f"        self.{param} = {ctype}()\n"
                res += f"        self.{param}.from_binary(data[{count}:{count + STRUCT_SIZES[ctype]}]) \n"
                count += STRUCT_SIZES[ctype]
            else:
                ctype_s = struct_symbols[ctype]
                res += f"        self.{param} = struct.unpack('<{ctype_s[Lang.STRUCT_PACK.value]}', data[{count}:{count + ctype_s[Lang.SIZE.value]}])[0]\n"
                count += ctype_s[Lang.SIZE.value]
        res += "\n"

        # get_values -----------------------------------------------------------------

        res += "    def get_values(self):\n"
        res += "        return (\n"
        for param, _ in items:
            res += f"            self.{param},\n"
        res += "        )\n\n"

        # get_binary ------------------------------------------------------------------

        res += "    def get_binary(self):\n"
        res += "        data = bytearray()\n"
        for param, ctype in items:
            type_check(ctype, obj)
            if ctype not in struct_symbols.keys():
                res += f"        data += self.{param}.get_binary()\n"
            else:
                res += f"        data += struct.pack('<{struct_symbols[ctype][Lang.STRUCT_PACK.value]}', self.{param})\n"
        res += "        return data\n\n"
    return res

def generate_c_structs():
    res = ""
    for obj in AST:
        items = list(obj.items())[1:]
        res += "typedef struct __attribute__((packed)) {\n"
        for param, ctype in items:
            type_check(ctype, obj)
            if ctype not in struct_symbols.keys():
                res += f"    {ctype} {param};\n"
            else:
                res += f"    {struct_symbols[ctype][Lang.C.value]} {param};\n"
        res += "}" + f" {obj['obj']};\n\n"
    return res
    

def main():
    parser = argparse.ArgumentParser(description="Generate Python and C structs from proto file.")
    parser.add_argument("input_file", nargs="?", default="proto.ppb",
                        help="Input proto file (default: proto.ppb)")
    parser.add_argument("-py", "--python_output", default="./proto.py",
                        help="Python output file (default: ./proto.py)")
    parser.add_argument("-c", "--c_output", default="./proto.h",
                        help="C header output file (default: ./proto.h)")
    args = parser.parse_args()

    parse_input(args.input_file)

    with open(args.python_output, "w") as fl:
        fl.write(generate_python_classes())

    with open(args.c_output, "w") as fl:
        fl.write(generate_c_structs())

if __name__ == "__main__":
    main()