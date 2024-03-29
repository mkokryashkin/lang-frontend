#ifndef TREE_H
#define TREE_H

#include <cstring>
#include <map>

#include "string_funcs.h"

std::map<int, char *> types = {{0,   "PROGRAM_ROOT"},
                               {1,   "DECLARATION"},
                               {2,   "FUNCTION"},
                               {3,   "VARLIST"},
                               {4,   "ID"},
                               {5,   "BLOCK"},
                               {6,   "IF"},
                               {7,   "WHILE"},
                               {8,   "OP"},
                               {9,   "EXPRESSION"},
                               {10,  "VAR"},
                               {11,  "RETURN"},
                               {12,  "INPUT"},
                               {13,  "OUTPUT"},
                               {14,  "INITIALIZE"},
                               {15,  "ASSIGNMENT"},
                               {16,  "NUMBER"},
                               {17,  "OPERATOR"},
                               {18,  "CALL"},
                               {19,  "C"},
                               {100, "ADD"},
                               {101, "SUB"},
                               {102, "MUL"},
                               {103, "DIV"},
                               {104, "SIN"},
                               {105, "COS"},
                               {106, "TAN"},
                               {107, "SQR"},
                               {200, "EQUAL"},
                               {201, "ABOVE"}};

template<class Node_T>
class Node {
 public:
  Node_T data;
  Node<Node_T> *left;
  Node<Node_T> *right;
  Node<Node_T> *parent;
  int type;

  Node (Node_T value, int node_type)
  {
    left = nullptr;
    right = nullptr;
    parent = nullptr;
    data = value;
    type = node_type;
  }
};

std::string_view serialize (Node<std::string_view *> *node)
{
  if (node)
    {
      if (node->data) return *(node->data);
      return std::string_view (types[node->type]);
    }
  return std::string_view ("@");
}

void skipSpaces (char **buffer)
{
  while (**buffer == ' ')
    {
      (*buffer)++;
    }

}

template<class T>
class Tree {
 private:
  size_t n_nodes = 0;

  Node<std::string_view *> *loadNode (char **buffer)
  {
    Node<std::string_view *> *node = nullptr;
    size_t n_symbols = 0;
    skipSpaces (buffer);
    if (**buffer == '{')
      {
        (*buffer)++;
        skipSpaces (buffer);
        sscanf (*buffer, "%*s%n", &n_symbols);
        std::string_view *data = new std::string_view (*buffer, n_symbols);

        (*buffer) += n_symbols;
        (*buffer)++;
        skipSpaces (buffer);

        auto node = new Node<std::string_view *> (data, 0);
        if (*data == "@") node = nullptr;
        if (**buffer == '}')
          {
            (*buffer)++;
            return node;
          }

        node->left = loadNode (buffer);
        node->right = loadNode (buffer);

        if (node->right) node->right->parent = node;
        if (node->left) node->left->parent = node;
        skipSpaces (buffer);

        if (**buffer == '}')
          {
            (*buffer)++;
            return node;
          }
        return nullptr;
      }
    return nullptr;
  }

  void dumpSubTree (Node<T> *node, std::ofstream &dump_file)
  {
    dump_file << "node" << node << "[label=\"{{" << node << "}|{TYPE|" << types[node->type] << "}|{VALUE|"
              << serialize (node)
              << "}|{LEFT|" << node->left
              << "}|{RIGHT|" << node->right << "}|{PARENT|" << node->parent << "}}}\",shape=record];" << std::endl;
    if (node->parent)
      {
        dump_file << "node" << node->parent << " -> node" << node << ";" << std::endl;
      }
    if (node->left) dumpSubTree (node->left, dump_file);
    if (node->right) dumpSubTree (node->right, dump_file);
  }

  void writeSubTreeToFile (Node<T> *node, std::ofstream &file)
  {
    file << serialize (node);
    if (node)
      {
        if (node->left || node->right)
          {
            file << " { ";
            writeSubTreeToFile (node->left, file);
            file << " } { ";
            writeSubTreeToFile (node->right, file);
            file << " }";
          }
      }
  }

 public:
  Node<T> *root = nullptr;

  Node<T> *search (Node<T> *node, T data)
  {
    if (strcmp (node->data, data) == 0) return node;
    Node<T> *left_subtree_node;
    Node<T> *right_subtree_node;
    if (node->left) left_subtree_node = search (node->left, data);
    else left_subtree_node = nullptr;
    if (node->right) right_subtree_node = search (node->right, data);
    else right_subtree_node = nullptr;

    if (right_subtree_node) return right_subtree_node;
    if (left_subtree_node) return left_subtree_node;
    return nullptr;
  }

  void createRoot (const T value, int type)
  {
    root = new Node<T> (value, type);
    n_nodes++;
  }

  Node<T> *getRoot ()
  {
    return root;
  }

  Node<T> *newNode (const T value, int type)
  {
    return new Node<T> (value, type);
  }

  void connectNodeLeft (Node<T> *parent, Node<T> *child)
  {
    parent->left = child;
    child->parent = parent;
    n_nodes++;
  }

  void connectNodeRight (Node<T> *parent, Node<T> *child)
  {
    parent->right = child;
    child->parent = parent;
    n_nodes++;
  }

  void deleteSubTree (Node<T> *subTreeRoot)
  {
    assert (subTreeRoot);

    if (subTreeRoot->parent)
      {
        if (subTreeRoot == subTreeRoot->parent->right) subTreeRoot->parent->right = nullptr;
        else if (subTreeRoot == subTreeRoot->parent->left) subTreeRoot->parent->left = nullptr;
        else perror ("Parental link broken");
        return;
      }

    if (subTreeRoot->right) deleteSubTree (subTreeRoot->right);
    if (subTreeRoot->left) deleteSubTree (subTreeRoot->left);

    delete subTreeRoot;
  }

  void saveToFile (char *filename)
  {
    std::ofstream file;
    file.open (filename);
    file << "{";
    writeSubTreeToFile (root, file);
    file << "}";
    file.close ();
  }

  void loadFromFile (char *filename)
  {
    File file{};
    file = loadFile (filename);
    char *buffer = file.raw_data;

    root = loadNode (&buffer);
  }

  void dump (const char *filename)
  {

    std::ofstream dump_file;
    dump_file.open (filename);
    dump_file << "digraph{" << std::endl;
    dumpSubTree (root, dump_file);
    dump_file << "}" << std::endl;
    dump_file.close ();
  }

  size_t countNodes (Node<T> *node)
  {
    size_t n_left_subtree = 0;
    size_t n_right_subtree = 0;
    if (node->left) n_left_subtree = countNodes (node->left);
    if (node->right) n_right_subtree = countNodes (node->right);
    return 1 + n_left_subtree + n_right_subtree;
  }

  void fixTypes (Node<std::string_view *> *subtree_root)
  {
    if (subtree_root)
      {
        if (*(subtree_root->data) == "PROGRAM_ROOT") subtree_root->type = PROGRAM_ROOT;
        else if (*(subtree_root->data) == "DECLARATION") subtree_root->type = DECLARATION;
        else if (*(subtree_root->data) == "FUNCTION") subtree_root->type = FUNCTION;
        else if (*(subtree_root->data) == "VARLIST") subtree_root->type = VARLIST;
        else if (*(subtree_root->data) == "BLOCK") subtree_root->type = BLOCK;
        else if (*(subtree_root->data) == "OP") subtree_root->type = OP;
        else if (*(subtree_root->data) == "INITIALIZE") subtree_root->type = INITIALIZE;
        else if (*(subtree_root->data) == "IF") subtree_root->type = IF;
        else if (*(subtree_root->data) == "ABOVE") subtree_root->type = ABOVE;
        else if (*(subtree_root->data) == "EQUAL") subtree_root->type = EQUAL;
        else if (*(subtree_root->data) == "C") subtree_root->type = C;
        else if (*(subtree_root->data) == "WHILE") subtree_root->type = WHILE;
        else if (*(subtree_root->data) == "RETURN") subtree_root->type = RETURN;
        else if (*(subtree_root->data) == "ASSIGNMENT") subtree_root->type = ASSIGNMENT;
        else if (*(subtree_root->data) == "MUL") subtree_root->type = MUL;
        else if (*(subtree_root->data) == "DIV") subtree_root->type = DIV;
        else if (*(subtree_root->data) == "ADD") subtree_root->type = ADD;
        else if (*(subtree_root->data) == "SUB") subtree_root->type = SUB;
        else if (*(subtree_root->data) == "CALL") subtree_root->type = CALL;
        else if (*(subtree_root->data) == "SQR") subtree_root->type = SQR;
        else if (*(subtree_root->data) == "INPUT") subtree_root->type = INPUT;
        else if (*(subtree_root->data) == "OUTPUT") subtree_root->type = OUTPUT;
        else if (isdigit ((*(subtree_root->data))[0])) subtree_root->type = NUMBER;
        else subtree_root->type = ID;

        if (subtree_root->left) fixTypes (subtree_root->left);
        if (subtree_root->right) fixTypes (subtree_root->right);
      }
  }

  void fixBlock (Node<std::string_view *> *subtree_root)
  {
    if (subtree_root->type == BLOCK)
      {
        auto op_node = new Node<std::string_view *> (nullptr, OP);
        if (subtree_root->left)
          {
            op_node->left = subtree_root->left;
            op_node->left->parent = op_node;
            subtree_root->left = nullptr;
          }
        if (subtree_root->right)
          {
            op_node->right = subtree_root->right;
            op_node->right->parent = op_node;
            subtree_root->right = nullptr;
          }
        subtree_root->right = op_node;
        subtree_root->right->parent = subtree_root;
      }
    if (subtree_root->left) fixBlock (subtree_root->left);
    if (subtree_root->right) fixBlock (subtree_root->right);
  }

};
#endif