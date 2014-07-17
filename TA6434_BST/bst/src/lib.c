// Implementation file for the lib module.

#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdarg.h>

#include "lib.h"

//
// strcmp compares the two strings s1 and s2
// strcmp(s1, s2):
//  s1 < s2 => return -ve
//  s1 > s2 => return +ve
//  s1 == s2 => return 0
//
// atol converts the initial portion of the string pointed to by s1 to long
// atoi(x):
// "67jjjkk" => 67
// "jjjkk67" => 0
//
int less( char *s1, char *s2 ) {
    if ( !atol(s1) )
        return ( (strcmp(s1, s2) < 0) ? 1 : 0 );
    else
        return ( atol(s1) < atol(s2 ));
}

int equal( char *s1, char *s2 ) {
    if ( !atol(s1) )
        return ( (strcmp(s1, s2) == 0) ? 1 : 0 );
    else
        return ( atol(s1) == atol(s2) );
}

//
// Search for a key in the tree.
// Return NULL if not found.
// Otherwise, return a pointer to the node containing key.
//
struct node *find ( struct tree *t, char *name ) {
    struct node *it = t->root;

    while ( it != NULL ) {
        if ( equal(it->name, name) )
            return it;
        else {
            int dir = less( it->name, name );
            it = it->link[dir];
        }
    }
    return NULL;
}

//
// Allocate memory to a new node, assigns the data, and
// sets all links to NULL.
// Return NULL if out of memory.
// Otherwise, return a pointer to the node.
//
struct node *make_node( char *name, int age, char *phone ) {
    struct node *temp;

    if ( ( temp = (struct node *) malloc( sizeof(struct node) ) ) == NULL ) {
        perror( "malloc failed" );
        return NULL;
    }

    strncpy(temp->name, name, sizeof(temp->name));
    temp->name[sizeof(temp->name)-1] = '\0';
    temp->age = age;
    memcpy(temp->phone, phone, sizeof(temp->phone));
    temp->phone[sizeof(temp->phone)-1] = '\0';
    temp->link[0] = temp->link[1] = NULL;

    return temp;
}

//
// Non-recursive binary search tree insertion.
// Return 0 if contain duplicate.
// Otherwise, return 1, with tree size updated.
//
int insert ( struct tree *t, char *name, int age, char *phone ) {
    if ( t->root == NULL ) {
        if ( ( t->root = make_node (name, age, phone) ) != NULL )
            t->size++;
    }
    else {
        struct node *it = t->root;
        int dir;

        for ( ; ; ) {
            dir = less( it->name, name );

            if ( equal(it->name, name) )
                return (0);
            else if ( it->link[dir] == NULL )
                break;

            it = it->link[dir];
        }
        if ( ( it->link[dir] = make_node (name, age, phone) ) != NULL )
            t->size++;
    }
    return (1);
}

//
// Remove a node containing key from the binary search tree.
// Return 0 if leaf is found.
// Otherwise, return 1 with tree size updated.
//
int remove_node ( struct tree *t, char *name ) {
    if ( t->root != NULL ) {
        struct node *p = NULL, *succ;
        struct node *it = t->root;
        int dir;

        for ( ; ; ) {
            if ( it == NULL )
                return (0);
            else if ( equal(it->name, name))
                break;

            dir = less(it->name, name);
            p = it;
            it = it->link[dir];
        }

        if ( it->link[0] != NULL && it->link[1] != NULL ) {
            p = it;
            succ = it->link[1];

            while ( succ->link[0] != NULL ) {
                p = succ;
                succ = succ->link[0];
            }

            strncpy(it->name, succ->name, sizeof(it->name));
            it->name[sizeof(it->name)-1] = '\0';
            it->age = succ->age;
            strncpy(it->phone, succ->phone, sizeof(it->phone));
            it->phone[sizeof(it->phone)-1] = '\0';

            p->link[p->link[1] == succ] = succ->link[1];
            free ( succ );
        }
        else {
            dir = it->link[0] == NULL;

            if ( p == NULL )
                t->root = it->link[dir];
            else
                p->link[p->link[1] == it] = it->link[dir];

            free ( it );
        }
        t->size--;
    }
    return (1);
}

//
// Recursively remove every node and
// free its memory.
//
void destroy_r ( struct node *root ) {
    if ( root != NULL ) {
        destroy_r ( root->link[0] );
        destroy_r ( root->link[1] );
        free ( root );
    }
}

//
// Wrapper for destroy_r function.
//
void destroy ( struct tree *tree ) {
    destroy_r ( tree->root );
}

//
// Recursively do binary search tree insertion.
// Phone string is placed on the second argument
// of insert function instead of name
// to print entire phone list in ascending order
// later by inorder function.
//
void treecpy_p( struct node *root, struct tree *t ) {
    if( root != NULL  ) {
        insert(t, root->phone, root->age, root->name);
        treecpy_p(root->link[0], t);
        treecpy_p(root->link[1], t);
    }
}

//
// Print both to stdout and output file
//
void tee( FILE *f_out, char const *fmt, ... ) {
    va_list ap;
    if ( opt.verbose ) {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
    va_start(ap, fmt);
    vfprintf(f_out, fmt, ap);
    va_end(ap);
}

//
// Recursively print every node.
//
void print_tree( struct node *root ) {
    if( root != NULL ) {
        printf("%s %d %s\n", root->name, root->age, root->phone);
        print_tree( root->link[0] );
        print_tree( root->link[1] );
    }
}

void write_node(FILE *f_out, struct node *root, int k){
    if ( k == 0 ) // default
        tee(f_out, "%s %d %s\n", root->name, root->age, root->phone);
    else if ( k == 1 ) {
        int v = opt.verbose;
        opt.verbose = 0;
        printf("%s\n", root->name);
        tee(f_out, "%s %d %s\n", root->phone ,root->age, root->name);
        opt.verbose = v;
    }
}

//
// Non-recursively traverse the binary tree,
// printing node values inorder.
//
int inorder ( char *buffer, struct tree *t, int k ) {
    struct node *it = t->root;
    struct node *up[(t->size)-1];
    int top = 0;
    FILE *f_out;

    if( ( f_out = fopen( (char *) buffer, "wb+" ) ) == NULL ) {
        perror( "fopen failed" );
        printf( "Could not create \"%s\".\n", buffer );
        return( 1 );
    }

    while ( it != NULL ) {
        while ( it != NULL ) {
            if ( it->link[1] != NULL )
                up[top++] = it->link[1];

            up[top++] = it;
            it = it->link[0];
        }

        it = up[--top];

        while ( top != 0 && it->link[1] == NULL ) {
            write_node(f_out, it, k);
            it = up[--top];
        }

        write_node(f_out, it, k);

        if ( top == 0 )
            break;

        it = up[--top];
    }

    fclose( f_out  );
    return( 0 );
}

//
// Load tree data from input file.
//
int load_tree( FILE *f_in, struct tree *t ) {
    char buffer [256], name[MAX_BUF], phone[MAX_BUF];
    int line=0, age;

    while(fgets(buffer, 256, f_in) != NULL){
        line++;
        if (sscanf(buffer, "%s %d  %s", name, &age, phone) == 3) {
            if ( strlen(name) > 32 ) {
                printf("Error in line %d: name has length "
                       "more than 32 chars\n", line);
                return(-1);
            }

            if ( age < 1 || age > 1000 ) {
                printf("Error in line %d: invalid age : %d\n",
                       line, age);
                return(-1);
            }

            if ( !check_phone(phone) ) {
                printf("Error in line %d: invalid phone number "
                       "specified: %s\n", line, phone);
                return(-1);
            }

            if ( !insert(t, name, age, phone) )
                printf("Warning in line %d: duplicate string. "
                       "Ignoring \"%s\".\n", line, name);
        }
        else {
            printf("Error in line %d: expected from input file "
                   "data in the form: <name> <age> <phone>\n", line);
            return(-1);
        }
    }

    return(0);
}

//
// Check phone number of the following pattern:
// 01234567891
// 0123456789
// 0312345678
// 061234567
//
int check_phone( char *phone ) {
    char buffer[MAX_PHONE+1];
    int ret_val = 1;
    size_t len = strlen(phone), pl = atol(phone);

    if ( (len != 9 && len != 10 && len != 11) ||
         (pl == 0) || (phone[0] != '0') )
        ret_val = 0;

    snprintf( (char *)buffer, sizeof( buffer ), "0%ld", pl );

    if ( len != strlen(buffer) )
        ret_val = 0;

    if (!ret_val)
        printf("\n[!] Error: invalid phone number "
               "specified: %s\n", phone);

    return (ret_val);
}

//
// Check input number from stdin.
//
int prompt_num(char *str, char *str_var, int *n, int min, int max){
    char buf[MAX_BUF], *tmp;
    size_t len;

    printf( str );

    if ( !fgets(buf, MAX_BUF, stdin) ) { // ctrl-d
        printf("\n");
        return(-1);
    }

    len = strlen(buf);
    if (buf[len-1] == '\n')
        buf[len-1] = '\0';

    *n = strtol(buf, &tmp, 0);
    if (*n < min || *n > max || *tmp != '\0' ) {
        printf("\n[!] Error: %s must be %d-%d: got \"%s\"\n", str_var, min, max, buf);
        return(0);
    }
    return(1);
}

//
// Check input string from stdin.
//
int prompt_str(char *str, char *str_var, char *buf, int min, int max){
    size_t len;

    printf( str );

    if ( !fgets(buf, MAX_BUF, stdin) ){ // ctrl-d
        printf("\n");
        return(-1);
    }
    len = strlen(buf);
    if (buf[len-1] == '\n')
        buf[len-1] = '\0';

    if ( len <= min || len > max ) {
        printf("\n[!] Error: invalid %s specified: \"%s\"\n", str_var, buf);
        return(0);
    }
    return(1);
}

//
// Cleanup routine.
//
void cleanup(FILE *f_in, FILE *f_out, struct tree *t){
    if ( t->root != NULL )
        destroy( t );
    if ( f_out != NULL )
        fclose( f_out );
    if ( f_in != NULL )
        fclose( f_in  );
}
