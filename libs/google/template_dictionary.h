// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ---
// Author: Craig Silverstein
//
// Based on the 'old' TemplateDictionary by Frank Jernigan.
//
// A template dictionary maps names (as found in template files)
// to their values.  There are three types of names:
//   variables: value is a string.
//   sections: value is a list of sub-dicts to use when expanding the section;
//             the section is expanded once per sub-dict.
//   template-include: value is a list of pairs: name of the template file
//             to include, and the sub-dict to use when expanding it.
// TemplateDictionary has routines for setting these values.
//
// For (many) more details, see the doc/ directory.


#ifndef TEMPLATE_TEMPLATE_DICTIONARY_H__
#define TEMPLATE_TEMPLATE_DICTIONARY_H__

#include <stdlib.h>      // for NULL
#include <string.h>      // for the strcmp() in streq
#include <stdarg.h>      // for StringAppendV()
#include <string>
#include <vector>
#include <google/ctemplate/hash_map.h>

namespace google {

// Most methods below take a TemplateString rather than a C++ string.
// This is for efficiency: it can avoid extra string copies.
// For any argument that takes a TemplateString, you can pass in any of:
//    * A C++ string
//    * A char*
//    * TemplateString(char*, length)
// The last of these is the most efficient, though it requires more work
// on the call site (you have to create the TemplateString explicitly).
class TemplateString {
 private:
  const char*  ptr_;
  size_t       length_;

 public:
  TemplateString(const char* s) : ptr_(s ? s : ""), length_(strlen(ptr_)) {}
  TemplateString(const std::string& s) : ptr_(s.data()), length_(s.size()) {}
  TemplateString(const char* s, size_t slen) : ptr_(s), length_(slen) {}
  TemplateString(const TemplateString& s) : ptr_(s.ptr_), length_(s.length_) {}
 private:
  TemplateString();    // no empty constructor allowed
  void operator=(const TemplateString&);   // or assignment

  friend class TemplateDictionary;         // only one who can read these
};


class TemplateDictionary {
 public:
  // name is used only for debugging.
  // arena is used to store all names and values.  It can be NULL (the
  //    default), in which case we create own own arena.
  explicit TemplateDictionary(const std::string& name,
                              class UnsafeArena* arena=NULL);
  ~TemplateDictionary();
  const std::string& name() const { return name_; }


  // --- Routines for VARIABLES
  // These are the five main routines used to set the value of a variable.
  // As always, wherever you see TemplateString, you can also pass in
  // either a char* or a C++ string, or a TemplateString(s, slen).

  void SetValue(const TemplateString variable, const TemplateString value);
  void SetIntValue(const TemplateString variable, int value);  // "%d" formatting
  void SetFormattedValue(const TemplateString variable, const char* format, ...)
#if 1
       __attribute__((__format__ (__printf__, 3, 4)))
#endif
      ;  // starts at 3 because of implicit 1st arg 'this'

  // The functor should be a class that defines
  //    string operator()(const string& in) const;
  // It takes 'value' as input and gives output that's actually stored
  // in the template.  Below we defined some useful escape-functors.
  // NOTE: This method is provided for convenience, but it's better to call
  //       SetValue() and do the escaping in the template itself when possible:
  //            "...{{MYVAR:html_escape}}..."
  template<class EscapeFunctor>
  void SetEscapedValue(const TemplateString variable, const TemplateString value,
                       EscapeFunctor escape_functor);

  // NOTE: This method is provided for convenience, but it's better to call
  //       SetFormattedValue() and do the escaping in the template itself:
  //            "...{{MYVAR:html_escape}}..."
  template<class EscapeFunctor>
  void SetEscapedFormattedValue(const TemplateString variable,
                                EscapeFunctor escape_functor,
                                const char* format, ...)
#if 1
       __attribute__((__format__ (__printf__, 4, 5)))
#endif
      ;  // starts at 4 because of implicit 1st arg 'this'

  // We also let you set values in the 'global' dictionary which is
  // referenced when all other dictionaries fail.  Note this is a
  // static method: no TemplateDictionary instance needed.  Since
  // this routine is rarely used, we don't provide variants.
  static void SetGlobalValue(const TemplateString variable,
                             const TemplateString value);

  // This is used for a value that you want to be 'global', but only
  // in the scope of a given template, including all its sections and
  // all its sub-included dictionaries.  The main difference between
  // SetTemplateGlobalValue() and SetValue(), is that
  // SetTemplateGlobalValue() values persist across template-includes.
  // This is intended for session-global data; since that should be
  // fairly rare, we don't provide variants.
  void SetTemplateGlobalValue(const TemplateString variable,
                              const TemplateString value);


  // --- Routines for SECTIONS
  // We show a section once per dictionary that is added with its name.
  // Recall that lookups are hierarchical: if a section tried to look
  // up a variable in its sub-dictionary and fails, it will look next
  // in its parent dictionary (us).  So it's perfectly appropriate to
  // keep the sub-dictionary empty: that will show the section once,
  // and take all var definitions from us.  ShowSection() is a
  // convenience routine that does exactly that.

  // Creates an empty dictionary whose parent is us, and returns it.
  // As always, wherever you see TemplateString, you can also pass in
  // either a char* or a C++ string, or a TemplateString(s, slen).
  TemplateDictionary* AddSectionDictionary(const TemplateString section_name);
  void ShowSection(const TemplateString section_name);

  // A convenience method.  Often a single variable is surrounded by
  // some HTML that should not be printed if the variable has no
  // value.  The way to do this is to put that html in a section.
  // This method makes it so the section is shown exactly once, with a
  // dictionary that maps the variable to the proper value.  If the
  // value is "", on the other hand, this method does nothing, so the
  // section remains hidden.
  void SetValueAndShowSection(const TemplateString variable,
                              const TemplateString value,
                              const TemplateString section_name);

  // NOTE: This method is provided for convenience, but it's better to call
  //       SetValueAndShowSection() and do the escaping in the template itself:
  //            "...{{MYVAR:html_escape}}..."
  template<class EscapeFunctor>
  void SetEscapedValueAndShowSection(const TemplateString variable,
                                     const TemplateString value,
                                     EscapeFunctor escape_functor,
                                     const TemplateString section_name);


  // --- Routines for TEMPLATE-INCLUDES
  // Included templates are treated like sections, but they require
  // the name of the include-file to go along with each dictionary.

  TemplateDictionary* AddIncludeDictionary(const TemplateString variable);

  // This is required for include-templates; it specifies what template
  // to include.  But feel free to call this on any dictionary, to
  // document what template-file the dictionary is intended to go with.
  void SetFilename(const TemplateString filename);


  // --- ESCAPE FUNCTORS
  // Some commonly-used escape functors.

  // Escapes < > " & <non-space whitespace> to &lt; &gt; &quot; &amp; <space>
  struct HtmlEscape { std::string operator()(const std::string&) const; };
  static HtmlEscape html_escape;

  // Escapes &nbsp; to &#160;
  struct XmlEscape { std::string operator()(const std::string&) const; };
  static XmlEscape xml_escape;

  // Escapes " ' \ <CR> <LF> <BS> to \" \' \\ \r \n \b
  struct JavascriptEscape { std::string operator()(const std::string&) const; };
  static JavascriptEscape javascript_escape;

  // Escapes " \ / <FF> <CR> <LF> <BS> <TAB> to \" \\ \/ \f \r \n \b \t
  struct JsonEscape { std::string operator()(const std::string&) const; };
  static JsonEscape json_escape;


  // --- DEBUGGING TOOLS

  // Logs the contents of a dictionary and its sub-dictionaries.
  // Dump goes to stdout/stderr, while DumpToString goes to the given string.
  // 'indent' is how much to indent each line of the output.
  void Dump(int indent=0) const;
  void DumpToString(std::string* out, int indent=0) const;

  // Indicate that annotations should be inserted during template expansion.
  // template_path_start - the start of a template path.  When
  // printing the filename for template-includes, anything before and
  // including template_path_start is elided.  This can make the
  // output less dependent on filesystem location for template files.
  void SetAnnotateOutput(const char* template_path_start);


 private:
  friend class SectionTemplateNode;   // for access to GetSectionValue(), etc.
  friend class TemplateTemplateNode;  // for access to GetSectionValue(), etc.
  friend class VariableTemplateNode;  // for access to GetSectionValue(), etc.
  friend class TemplateNode;          // for access to ShouldAnnotateOutput()
  friend class Template;              // for access to ShouldAnnotateOutput()
  friend class TemplateDictionaryUnittest;
  friend class TemplateSetGlobalsUnittest;

  // The dictionary types
  struct streq {
    bool operator()(const char* s1, const char* s2) const {
      return ((s1 == 0 && s2 == 0) ||
              (s1 && s2 && *s1 == *s2 && strcmp(s1, s2) == 0));
    }
  };
  typedef std::vector<TemplateDictionary *>  DictVector;
  typedef HASH_NAMESPACE::hash_map<const char*, const char*,
                                   HASH_NAMESPACE::hash<const char*>, streq>
     VariableDict;
  typedef HASH_NAMESPACE::hash_map<const char*, DictVector*,
                                   HASH_NAMESPACE::hash<const char*>, streq>
     SectionDict;
  typedef HASH_NAMESPACE::hash_map<const char*, DictVector*,
                                   HASH_NAMESPACE::hash<const char*>, streq>
     IncludeDict;
  // This is used only for global_dict_, which is just like a VariableDict
  typedef HASH_NAMESPACE::hash_map<const char*, const char*,
                                   HASH_NAMESPACE::hash<const char*>, streq>
     GlobalDict;

  // Constructor created for all children dictionaries. This includes
  // both a pointer to the parent dictionary and also the the
  // template-global dictionary from which all children (both
  // IncludeDictionary and SectionDictionary) inherit.  Values are
  // filled into global_template_dict via SetTemplateGlobalValue.
  explicit TemplateDictionary(const std::string& name,
                              class UnsafeArena* arena,
                              TemplateDictionary* parent_dict,
                              VariableDict* template_global_dict);

  // Helps set up the static stuff
  static GlobalDict* SetupGlobalDictUnlocked();

  // Utility functions for copying a string into the arena.
  const char *Memdup(const char* s, int slen);
  const char *Memdup(const TemplateString& s) {return Memdup(s.ptr_, s.length_);}

  // Used to do the formatting for the SetFormatted*() functions
  static int StringAppendV(char* space, char** out,
                           const char* format, va_list ap);

  // Was AnnotateOutput() called?  If so, what parameter was passed in?
  bool ShouldAnnotateOutput() const;
  const char* GetTemplatePathStart() const;

  // Needed because Template::Expand() and its children try to call these
  const char *GetSectionValue(const std::string& variable) const;
  bool IsHiddenSection(const std::string& name) const;
  const DictVector& GetDictionaries(const std::string& section_name) const;
  bool IsHiddenTemplate(const std::string& name) const;
  const char *GetIncludeTemplateName(const std::string& variable, int dictnum)
      const;
  const DictVector& GetTemplateDictionaries(const std::string& include_name) const;


  // The "name" of the dictionary for debugging output (Dump, etc.)
  // The arena, also set at construction time
  std::string const name_;
  class UnsafeArena* const arena_;
  bool should_delete_arena_;   // only true if we 'new arena' in constructor

  // The three dictionaries that I own -- for vars, sections, and template-incs
  VariableDict* variable_dict_;
  SectionDict* section_dict_;
  IncludeDict* include_dict_;

  // Like variable_dict_, but persists across template-includes.
  // Unlike the global dict in that only this template and its children
  // get to see the values.  So it's halfway in between.
  VariableDict* template_global_dict_;
  bool template_global_dict_owner_;  // true iff our class new-ed template_g_d_

  // My parent dictionary, used when variable lookups at this level fail.
  // Note this is only for *variables*, not sections or templates.
  TemplateDictionary* parent_dict_;
  // The static, global dictionary, at the top of the parent-dictionary chain
  static GlobalDict* global_dict_;

  // The filename associated with this dictionary.  If set, this declares
  // what template the dictionary is supposed to be expanded with.  Required
  // for template-includes, optional (but useful) for 'normal' dicts.
  const char* filename_;

  // If non-NULL, we want to annotate template vars at expand time (to debug).
  // If set to something other than the empty string, this string indicates
  // the end of the prefix to cut off when printing template-include filenames.
  const char* template_path_start_for_annotations_;

 private:
  // Can't invoke copy constructor or assignment operator
  TemplateDictionary(const TemplateDictionary&);
  void operator=(const TemplateDictionary&);
};


// Alas, template code needs to be defined in a .h file, not a .cc

template<class EscapeFunctor>
void TemplateDictionary::SetEscapedValue(TemplateString variable,
                                         TemplateString value,
                                         EscapeFunctor escape_functor) {
  std::string escval(escape_functor(std::string(value.ptr_, value.length_)));
  SetValue(variable, escval);
}

template<class EscapeFunctor>
void TemplateDictionary::SetEscapedFormattedValue(TemplateString variable,
                                                  EscapeFunctor escape_functor,
                                                  const char* format, ...) {
  char scratch[1024], *buffer;
  va_list ap;
  va_start(ap, format);
  const int buflen = StringAppendV(scratch, &buffer, format, ap);
  va_end(ap);

  std::string escaped_string(escape_functor(std::string(buffer, buflen)));
  SetValue(variable, escaped_string);
  if (buffer != scratch)
    delete[] buffer;
}

template<class EscapeFunctor>
void TemplateDictionary::SetEscapedValueAndShowSection(
    const TemplateString variable, const TemplateString value,
    EscapeFunctor escape_functor, const TemplateString section_name) {
  std::string escval(escape_functor(std::string(value.ptr_, value.length_)));
  if (escval.length() == 0)    // no value: the do-nothing case
    return;
  TemplateDictionary* sub_dict = AddSectionDictionary(section_name);
  sub_dict->SetValue(variable, escval);
}

}

#endif  // TEMPLATE_TEMPLATE_DICTIONARY_H__
