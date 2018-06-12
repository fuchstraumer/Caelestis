import CppHeaderParser
import sys
import argparse

def parseHeader(path):
    try:
        cppHeader = CppHeaderParser.CppHeader(path)
        return cppHeader
    except CppHeaderParser.CppParseError as e:
        print(e)
        sys.exit(1)

def collectPublicMembers(cpp_class):
    public_methods = cpp_class["methods"]["public"]
    public_variables = cpp_class["properties"]["public"]
    return [ public_methods, public_variables ]
    
def main():
    test_header = parseHeader("include/graph/PipelineSubmission.hpp")
    test_class = test_header.classes["PipelineSubmission"]
    public_items = collectPublicMembers(test_class)
    

if __name__ == "__main__":
    main()