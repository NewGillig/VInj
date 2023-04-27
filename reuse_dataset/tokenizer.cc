#include <iostream>
#include <experimental/filesystem>
#include <clang-c/Index.h>
#include <fstream>
namespace fs=std::experimental::filesystem;
using namespace std;

void DisplayTokens(const CXTranslationUnit& tu) {
  CXCursor cursor = clang_getTranslationUnitCursor(tu);
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXToken* tokens = NULL;
  unsigned num_tokens = 0;
  clang_tokenize(tu, range, &tokens, &num_tokens);
  for (unsigned i = 0; i < num_tokens; ++i) {
    const CXToken& token = tokens[i];
    CXTokenKind kind = clang_getTokenKind(token);
    CXString spelling_str = clang_getTokenSpelling(tu, token);
    CXSourceRange range = clang_getTokenExtent(tu, token);
    std::cerr << clang_getCString(spelling_str) << "!#@$";
              // << static_cast<int>(kind) << ", "
              // << "[" << range.begin_int_data
              // << ":" << range.end_int_data << "]"
              // << std::endl;
    clang_disposeString(spelling_str);
  }
  clang_disposeTokens(tu, tokens, num_tokens);
}

void WriteTokens(const CXTranslationUnit& tu, const std::string& filepath) {
  CXCursor cursor = clang_getTranslationUnitCursor(tu);
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXToken* tokens = NULL;
  unsigned num_tokens = 0;
  clang_tokenize(tu, range, &tokens, &num_tokens);

  ofstream file;
  file.open(filepath);

  for (unsigned i = 0; i < num_tokens; ++i) {
    const CXToken& token = tokens[i];
    CXTokenKind kind = clang_getTokenKind(token);
    CXString spelling_str = clang_getTokenSpelling(tu, token);
    CXSourceRange range = clang_getTokenExtent(tu, token);
    file << clang_getCString(spelling_str) << "!@#$";
    clang_disposeString(spelling_str);
  }
  clang_disposeTokens(tu, tokens, num_tokens);
  file.close();
}

void Tokenize(const std::string& filepath) {
  const int exclude_decls_from_pch = 1;
  const int display_diagnostics = 0;
  CXIndex index = clang_createIndex(exclude_decls_from_pch,
                                    display_diagnostics);
  const unsigned index_options = CXGlobalOpt_None;
  clang_CXIndex_setGlobalOptions(index, index_options);
  const char* command_line_args[] = { "-Xclang", "-cc1" };
  const int num_command_line_args = sizeof(command_line_args) / sizeof(char*);
  const unsigned num_unsaved_files = 0;
  CXUnsavedFile* unsaved_files = NULL;
  CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(
      index, filepath.c_str(), num_command_line_args, command_line_args,
      num_unsaved_files, unsaved_files);
  if (tu != NULL) {
    WriteTokens(tu,filepath);
    clang_disposeTranslationUnit(tu);
  } else {
    std::cerr << "Failed to tokenize: \"" << filepath << "\"" << std::endl;
  }
  clang_disposeIndex(index);
}

int main(int argc, char** argv) {
  if (argc < 2) {
     return 1;
  }
  string path=argv[1];
  for (const auto & entry : fs::directory_iterator(path))
  {
    std::string path2(entry.path());
    for (const auto & entry2 : fs::directory_iterator(path2))
    {
      cout << entry2.path()<<endl;
      std::string filepath(entry2.path());
      Tokenize(filepath);
    }
  }

  return 0;
}
