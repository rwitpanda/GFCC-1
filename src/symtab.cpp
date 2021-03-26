// AUM SHREEGANESHAAYA NAMAH||

/************************ NOTE TO THE DEVELOPER ************************
 * SEE <proj_root>/src/include/symtab.h for description of various classes,
   members, methods, etc.
 * Search "TODO" for things to do.
 * Search "ASSUMPTIONS" for assumptions.
 * Execution using: g++ [-DSYMTEST] [-DSYMDEBUG] symtab.cpp -Iinclude
************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <symtab.h>
#include <typo.h>

#ifdef DEBUG_SYM
const bool dbg = true;
#else
const bool dbg = false;
#endif

typedef unsigned long int ull;

using namespace std;


bool acceptType(ull type) {
  // analyze type properly. eg: return 0 if "auto static int"
  // TODO
  return true;
}

/*************************************/
/************ CLASS "sym" ************/
/*************************************/
// TODO: see how to prevent instantiation if acceptType(type) == false
sym::sym(string _name, ull _type) : name(_name), type(_type) {
  if (_name == "" || !acceptType(_type)) {
    if (dbg) msg(ERR) << "Invalid name \"" << _name << "\" or type \"" << _type << "\" passed!";
  }
}

void sym::dump(ofstream &f) {
  f << name << " , " << type << endl; // decode type later on
}


/****************************************/
/************ CLASS "symtab" ************/
/****************************************/
symtab::symtab() { } // do nothing - initialization in class declaration

symtab::symtab(string _name) : name(_name) { }

symtab::symtab(symtab *_parent) : parent(_parent) { }

symtab::symtab(string _name, symtab *_parent) : name(_name), parent(_parent) { }

symtab::~symtab() {
  int l1 = subScopes.size(), l2 = syms.size();
  for (int i = 0; i < l1; i++) {
    if (dbg) cout << "Deleting subscope \"" << subScopes[i]->name << "\"." << endl;
    delete subScopes[i];
  }
  for (int i = 0; i < l2; i++) {
    if (dbg) cout << "Deleting " << syms[i]->name << endl;
    delete syms[i];
  }
}

sym* symtab::srchSym(string _name) { // search symbol by name
  if (_name == "") return NULL; // not required, but a speedup.

  int l = syms.size();
  for (int i = 0; i < l; i++) {
    if (syms[i]->name == _name) return syms[i];
  }
  return NULL;
}

bool symtab::pushSym(sym* newSym) {
  if (!newSym || srchSym(newSym->name)) {
    if (dbg) msg(WARN) << "Could not push new symbol in current scope.";
    return false;
  }
  if (dbg) cout << "Pushing " << newSym->name << endl;
  syms.push_back(newSym);
  return true;
}

bool symtab::pushSym(string _name, ull _type) {
  if (_name == "" || !acceptType(_type) || srchSym(_name)) {
    if (dbg) msg(WARN) << "Could not push \"" << _name << "\" in current scope.";
    return false;
  }
  return pushSym(new sym(_name, _type));
  // sym* newSym = new sym(_name, _type);
  // if (!newSym) return false;
  // if (dbg) cout << "inserting " << _name << endl;
  // syms.push_back(newSym);
  // return true;
}

void symtab::dump(ofstream &f, string scopePath) {
  f << "###### Scope = " << scopePath << name << " ######," << endl;
  
  // First print symbols, then scopes; just a random decision.
  // CAUTION: Will NOT reflect the history of symbols in CSV file.
  int l1 = syms.size(), l2 = subScopes.size();
  for (int i = 0; i < l1; i++) syms[i]->dump(f); // ASSUMPTION: none is NULL
  for (int i = 0; i < l2; i++) subScopes[i]->dump(f, scopePath + name + " >> "); // ASSUMPTION: none is NULL  
}


/*****************************************/
/************ class "symRoot" ************/
/*****************************************/
symRoot::symRoot() {
  currScope = root = new symtab("Global"); // handle exception if returns NULL
  if (dbg) {
    if (!root) msg(WARN) << "Could not open global scope.";
    else cout << "Global scope opened succesfully." << endl;
  }
}

symRoot::~symRoot() {
  delete root;
}

bool symRoot::newScope() {
  if (!currScope) return false;
  return newScope( "_unnamed_" + to_string(currScope->subScopes.size()) );
}

bool symRoot::newScope(string scope_name) {
  if (!currScope) return false;
  symtab* new_scope = new symtab(scope_name, currScope);
  if (!new_scope) return false;
  currScope->subScopes.push_back(new_scope);
  currScope = new_scope;
  if (dbg) cout << "Opening new scope \"" << scope_name << "\"." << endl;
  return true;
}

void symRoot::closeScope() {
  if (currScope && (currScope != root)) {
    if (dbg) cout << "Closing existing scope \"" << currScope->name << "\"." << endl;
    currScope = currScope->parent;
  } else {
    if (dbg) msg(WARN) << "Cannot close global scope!";
  }
  // In future, can add functionality to delete currScope - NOT NOW.
  // Hence, "closing" and "deleting" scopes are two DIFFERENT things.
}

sym* symRoot::lookup(string _name) {
  if (!currScope) return NULL;  
  return currScope->srchSym(_name);
}

sym* symRoot::gLookup(string _name) {
  symtab* run_scope = currScope;

  while (run_scope) {
    sym* symb = run_scope->srchSym(_name);
    if (symb) return symb; // found
    run_scope = run_scope->parent; // check in parent
  }

  return NULL; // not found
}

bool symRoot::pushSym(sym* newSym) {
  if (!currScope) return false;
  return currScope->pushSym(newSym);
}

bool symRoot::pushSym(string _name, ull _type) {
  if (!currScope) return false;
  return currScope->pushSym(_name, _type);
}

void symRoot::dump(ofstream &f) {
  f << "###### Scope = Global ######," << endl;
  if (!root) return; // nothing to do
  root->dump(f, "");
}


/******************************************************************/
/************************ TEST SUITES HERE ************************/
/******************************************************************/

void testSym() {
  sym("main", 00);
  sym("", 00);
}

void testSymTab() {
  symtab _st;
  _st.srchSym("");
  _st.pushSym(new sym("praskr", 123));
  _st.pushSym("deba", 456);
  _st.pushSym(new sym("praskr", 123));
  _st.srchSym("padnda");
  _st.srchSym("deba");
  _st.srchSym("priyanag");
}

void testSymRoot() {
  symRoot _sr;
  _sr.pushSym("main", 0);
  _sr.newScope();
  _sr.pushSym("x", 45);
  _sr.lookup("x");
  _sr.lookup("y");
  _sr.pushSym("x", 45);
  _sr.newScope(); _sr.newScope("for_loop"); _sr.newScope();
  _sr.newScope(); _sr.newScope();
  _sr.newScope(); _sr.closeScope(); _sr.newScope(); _sr.closeScope();
  _sr.closeScope(); _sr.closeScope(); _sr.closeScope(); _sr.closeScope();
  _sr.closeScope(); _sr.closeScope(); _sr.closeScope(); _sr.closeScope();
  _sr.pushSym("main", 0);
  _sr.pushSym("maina", 0);
  
  ofstream f("out.csv");
  f << "# File Name: <must_get_somehow>" << endl << endl;
  f << "SYMBOL NAME , SYMBOL TYPE" << endl << endl;
  _sr.dump(f);
  f.close();
}

#ifdef TEST_SYM

int main() {
  testSym();
  testSymTab();
  testSymRoot();

  return 0;
}

#endif