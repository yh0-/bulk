// Interface for the lib module.

#ifndef LIB_H
#define LIB_H

#define MAX_NAME 32 // Assume max name is 32 chars wide
#define MIN_NAME 2
#define MAX_PHONE 11 // Assume max phone number is 11 chars
#define MAX_AGE 1000
#define MIN_AGE 1
#define MAX_BUF 128

struct options
{
    int verbose;
}
opt;

//
// Node for element of binary tree
// link[0] is the left subtree and
// link[1] is the right subtree.
//
struct node
{
    char name[MAX_NAME+1];
    int age;
    char phone[MAX_PHONE+1];
    struct node *link[2];
}
node;

// Root node of the binary tree.
struct tree
{
    size_t size;
    struct node *root;
}
tree;

// Function prototypes
int less( char *s1, char *s2 );
int equal( char *s1, char *s2 );
struct node *find ( struct tree *t, char *name );
struct node *make_node( char *name, int age, char *phone );
int insert ( struct tree *t, char *name, int age, char *phone );
int load_tree( FILE *f_in, struct tree *t );
void treecpy_p( struct node *root, struct tree *t );
int remove_node ( struct tree *t, char *name );
void destroy_r ( struct node *root );
void destroy ( struct tree *tree );
void print_tree( struct node *root );
void tee( FILE *f_out, char const *fmt, ... );
int inorder ( char *buffer, struct tree *t, int k );
int check_phone( char *phone );
int prompt_str(char *str, char *str_var, char *buf, int min, int max);
int prompt_num(char *str, char *str_var, int *n, int min, int max);
void cleanup(FILE *f_in, FILE *f_out, struct tree *t);
void write_node(FILE *f_out, struct node *root, int k);
#endif

