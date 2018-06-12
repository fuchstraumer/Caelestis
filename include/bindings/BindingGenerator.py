import CppHeaderParser
import sys
import argparse
import ntpath

def parseHeader(path):
    try:
        cppHeader = CppHeaderParser.CppHeader(path)
        return cppHeader
    except CppHeaderParser.CppParseError as e:
        print(e)
        sys.exit(1)

def collectPublicMembers(cpp_class):
    results = {}
    results["methods"] = cpp_class["methods"]["public"]
    results["variables"] = cpp_class["properties"]["public"]
    results["namespace"] = cpp_class["namespace"]
    return results

def extractAllFromHeader(path):
    header_file = parseHeader(path)
    header_dict = {}
    for obj in header_file.classes:
        header_dict[obj] = collectPublicMembers(header_file.classes[obj])
    return header_dict

def getHeaderGuard(path):
    result = ntpath.basename(path)
    result = result.replace('.', '_', 1)
    return result.upper()

def getIncludePath(path, output_location):
    # return relative path to given path, from current output location
    return path

def appendDefaultTemplate(header_parts):
    header_parts.append('template<typename T>')
    header_parts.append('inline void RegisterType(lua_State* state);')

def skipMember(member):
    if member["constructor"] or member["destructor"]:
        return True
    # Operators automatically registered by Sol2
    if member["operator"] is not False:
        return True

def getMethodCount(member_table):
    count = 0
    for method in member_table:
        if not skipMember(method):
            count += 1
    return count

def getConstructors(method_table):
    results = []
    for method in method_table:
        if method["constructor"] is True:
            constructor_string = '{}('.format(method["name"])
            i = 0
            for arg in method["parameters"]:
                constructor_string += arg["raw_type"]
                if i != (len(method["parameters"]) - 1):
                    constructor_string += ', '
                i += 1
            constructor_string = '{})'.format(constructor_string)          
            results.append(constructor_string)
    return results      

def addSingleMethod(member):
    method_string = '&{}::{}'.format(member["path"], member["name"])
    return '{}, {}'.format(member["name"], method_string)

def addSingleMember(member):
    member_str = '&{}::{}'.format(member["property_of_class"], member["raw_type"])
    return '{}, {}'.format(member["name"], member_str)

def generateSingleObjectBindings(object_name, object_dict, header_parts):
    true_object_name = '{}::{}'.format(object_dict["namespace"],object_name)
    header_parts.append('')
    header_parts.append('template<>')
    header_parts.append('inline void RegisterType<{}>(lua_State* state) {}'.format(true_object_name, '{'))
    header_parts.append('    sol::state_view solState(state);')
    header_parts.append('    solState.new_usertype<{}>(\"{}\",'.format(true_object_name,object_name))
    constructors = getConstructors(object_dict["methods"])
    if len(constructors) is not 0:
        for ctor in constructors:
            header_parts.append('    sol::constructors<{}>,'.format(ctor))

    for method in object_dict["methods"]:
        if not skipMember(method):
            member_str = addSingleMethod(method)
            header_parts.append('    {}'.format(member_str))
        
    for member in object_dict["variables"]:
        member_str = addSingleMember(member)
        header_parts.append('    {}'.format(member_str))

    header_parts.append('{}'.format('}'))
    header_parts.append('')
    

def generateHeader(path, output_location):
    guard_name = getHeaderGuard(path)
    header_dict = extractAllFromHeader(path)

    header_parts = []
    guard = 'VPSK_GENERATED_BINDINGS_{}'.format(guard_name)
    header_parts.append('#ifndef {}'.format(guard))
    header_parts.append('#define {}'.format(guard))
    header_parts.append(' ')
    # Include header we are parsing
    header_parts.append('#include \"{}\"'.format(getIncludePath(path, output_location)))
    header_parts.append('#include \"sol.hpp\"')
    header_parts.append('')
    header_parts.append('')
    appendDefaultTemplate(header_parts)
    for entry in header_dict:
        generateSingleObjectBindings(entry, header_dict[entry], header_parts)
    header_parts.append('#endif //!{}'.format(guard))

    return '\n'.join(header_parts)

def main():
    generateHeader("include/graph/PipelineResource.hpp", "include/bindings")

if __name__ == "__main__":
    main()