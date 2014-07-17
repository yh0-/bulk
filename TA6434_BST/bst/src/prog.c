/*
*  prog.c
*  A program to test binary search tree (BST)
*
*  Program created by Yahya Sjahrony on 14/10/13
*  TTTA6434 ALGORITHM AND DATA STRUCTURE
*
*  The program that reads a list of names, ages and telephone numbers
*  from a text file <input file> and inserts them into a Binary Search
*  Tree (BST).
*  The input text file is expected to contain list in the form of:
*  <name> <age> <phone>.
*  Once the tree has been built, the program will present the user with
*  a menu that allows him or her to search the list for a specified name,
*  insert a new name, delete an existing name, or print the entire phone
*  list in ascending order.
*
*  The program menu has the following choices:
*    1. search the list for a specified name
*    2. insert a new name
*    3. delete an existing name
*    4. print entire phone list in ascending order
*    5. print current inorder tree
*    6. quit
*
*  At the end, the data in the list is written back to the file with
*  file name <input file>-out.
*
*  Test input file with 10 names is included with this program (input.txt).
*
*  Reference:
*  http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_bst1.aspx
*
*  To build the program executable, execute the following command:
*  make
*  Then, run:
*  ./prog
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include<string.h>

#include "lib.h"

char usage[] =

"\n"
"  prog - Binary search tree program\n"
"\n"
"  usage: %s [options] <input file>\n"
"\n"
"  Options:\n"
"\n"
"      -v, --verbose    : verbose (print more messages)\n"
"      -h, --help       : displays this usage screen\n"
"\n"
"  Note: input file should contain list of <name> <age> <phone>\n"
"\n";

char menu[] =

"\n"
" ********************** \n"
" ***  Program Menu  *** \n"
" ********************** \n"
" 1 - Search the list for a specified name.\n"
" 2 - Insert a new name.\n"
" 3 - Delete an existing name.\n"
" 4 - Print the entire phone list in ascending order.\n"
" 5 - Print current inorder tree.\n"
" 6 - Quit.\n"
" Enter choice:"
"\n ";

int main ( int argc, char *argv[] ) {
    char buf[MAX_BUF], phonebuf[MAX_BUF], buffer[65536];
    int n = 0, age = 0, ret_val = 0, v;
    FILE *f_in, *f_out;

    struct tree *tn, *tp;
    struct node *nd;

    // Check the arguments
    memset( &opt, 0, sizeof( opt ) );

    while( 1 ) {
        int option_index = 0;

        static struct option long_options[] = {
            {"verbose",     0, 0, 'v'},
            {"help",        0, 0, 'h'},
            {0,             0, 0,  0 }
        };

        int option = getopt_long( argc, argv,
                        "vh",
                        long_options, &option_index );

        if( option < 0 )
            break;

        switch( option ) {
            case 0 :
                break;

            case ':' :
                printf("\"%s --help\" for help.\n", argv[0]);
                return( 1 );

            case '?' :
                printf("\"%s --help\" for help.\n", argv[0]);
                return( 1 );

            case 'v' :
                opt.verbose = 1;
                break;

            case 'h' :
	        printf( usage, argv[0] );
                return( 1 );

            default : goto usage;
        }
    }

    if( argc - optind != 1 ) { // getopt.h
        if(argc == 1) {
usage:
	    printf( usage, argv[0] );
        }
        if( argc - optind == 0)
            printf("No input file specified.\n");
        if(argc > 1)
            printf("\"%s --help\" for help.\n", argv[0]);
        return( 1 );
    }

    if( ( f_in = fopen( argv[optind], "rb" ) ) == NULL ) {
        perror( "fopen failed\n" );
        printf( "Could not open \"%s\".\n", argv[optind] );
        return (1);
    }

    // Get output filename
    n = strlen( argv[optind] );

    if( n > 4 && ( n + 5 < (int) sizeof( buffer ) ) &&
        argv[optind][n - 4] == '.' )
    {
        memcpy( buffer , argv[optind], n - 4 );
        memcpy( buffer  + n - 4, "-out", 4 );
        memcpy( buffer  + n, argv[optind] + n - 4, 5 );
    }
    else
    {
        if( n > 5 && ( n + 6 < (int) sizeof( buffer ) ) &&
            argv[optind][n - 5] == '.' )
        {
            memcpy( buffer , argv[optind], n - 5 );
            memcpy( buffer  + n - 5, "-out", 4 );
            memcpy( buffer  + n - 1, argv[optind] + n - 5, 6 );
        }
        else
        {
            memset( buffer , 0, sizeof( buffer ) );
            snprintf( (char *) buffer , sizeof( buffer ) - 1,
                      "%s-out", argv[optind] );
        }
    }

    if ( (tn = (struct tree *)malloc( sizeof(struct tree) ) )
          == NULL) {
        perror( "malloc failed" );
        fclose( f_in  );
        return( 1 );
    }
    tn->size = 0;
    tn->root = NULL;

    if ( load_tree(f_in, tn) != 0 ) {
        fclose( f_in  );
        return( 1 );
    }

    if ( tn->root == NULL ) {
        printf( "Something went wrong with input file "
                "\"%s\".\n", argv[optind] );
        fclose( f_in  );
        return( 1 );
    }

    if( ( f_out = fopen( (char *) buffer, "wb+" ) ) == NULL ) {
        perror( "fopen failed" );
        printf( "Could not create \"%s\".\n", buffer );
        fclose( f_in  );
        return( 1 );
    }

    while (1) {
        ret_val = prompt_num( menu, "choice", &n, 1, 6);
        if ( ret_val == 0 )
            continue;
        else if ( ret_val == -1 )
            break;

        if ( n == 6 )
            break;
        else if ( n >= 1 && n <=3 ) {
            ret_val = prompt_str(" Enter name: ", "name", buf, 2, MAX_NAME);
            if ( ret_val == 0 )
                continue;
            else if ( ret_val == -1 )
                break;
        }

        switch( n ) {
            case 1 :
                // Search node into tree
                nd = find(tn, buf);
                if (!nd) {
                    printf("\n[!] Data not found in tree: %s\n", buf);
                    continue;
                }
                else {
                    printf("\n[+] Searched node: \n\nName: %s "
                           "Age: %d Phone: %s\n",
                           nd->name, nd->age,
                           nd->phone);
                }

                v = opt.verbose;
                opt.verbose = 0;
                if ( inorder(buffer, tn, 0) != 0 ) {
                    cleanup(f_in, NULL, tn);
                    return( 1 );
                }
                opt.verbose = v;
                if (!opt.verbose)
                    printf ("\n[+] The list is saved on \"%s\"\n", buffer);

                break;

            case 2 :
                ret_val = prompt_num(" Enter age: ", "age", &age, 1, MAX_AGE);
                if ( ret_val == 0 )
                    continue;
                else if ( ret_val == -1 ) {
                    cleanup(f_in, f_out, tn);
                    return(1);
                }

                ret_val = prompt_str(" Enter phone: ", "phone", phonebuf, 0, MAX_PHONE);
                if ( ret_val == 0 )
                    continue;
                else if ( ret_val == -1 ) {
                    cleanup(f_in, f_out, tn);
                    return(1);
                }

                if (!check_phone(phonebuf))
                    continue;

                // Insert a new node
                if ( insert(tn, buf, age, phonebuf) )
                    printf ("\n[+] The list is saved on \"%s\"\n",
                            buffer);
                else
                    printf("\n[!] Cannot insert \"%s\" in tree: "
                           "duplicate string.\n", buf);

                v = opt.verbose;
                opt.verbose = 0;
                if ( inorder(buffer, tn, 0) != 0 ) {
                    cleanup(f_in, NULL, tn);
                    return( 1 );
                }
                opt.verbose = v;
                if (!opt.verbose)
                    printf ("\n[+] The list is saved on \"%s\"\n", buffer);

                break;

            case 3 :
                nd = find(tn, buf);
                if (!nd) {
                    printf("\n[!] Data not found in tree: %s\n", buf);
                    continue;
                }

                // Delete an existing node
                remove_node(tn, buf);

                v = opt.verbose;
                opt.verbose = 0;
                if ( inorder(buffer, tn, 0) != 0 ) {
                    cleanup(f_in, NULL, tn);
                    return( 1 );
                }
                opt.verbose = v;
                if (!opt.verbose)
                    printf ("\n[+] The list is saved on \"%s\"\n", buffer);
                else
                    printf ("\n[+] Done!\n");
                break;

            case 4 :
                if ( ! ( tp = (struct tree *)malloc(sizeof(struct tree)) ) ) {
                    perror( "malloc failed" );
                    cleanup(f_in, NULL, tn);
                    return( 1 );
                }
                tp->size = 0;
                tp->root = NULL;
                treecpy_p(tn->root, tp);

                printf("\n[+] Phone list in ascending order (%ld):\n",
                       tp->size);

                if ( inorder(buffer, tp, 1) != 0 ) {
                    destroy( tp );
                    cleanup(f_in, NULL, tn);
                    return( 1 );
                }

                if (!opt.verbose)
                    printf ("\n[+] The list is saved on \"%s\"\n", buffer);

                destroy( tp );
                break;

            case 5 :
                printf("\n[+] Inorder traversal (%ld):\n", tn->size);
                v = opt.verbose;
                opt.verbose = 1;
                if ( inorder(buffer, tn, 0) != 0 ) {
                    cleanup(f_in, NULL, tn);
                    return( 1 );
                }
                opt.verbose = v;
                if (!opt.verbose)
                    printf ("\n[+] The list is saved on \"%s\"\n", buffer);
                break;
        }
    }
    cleanup(f_in, f_out, tn);
    return( 0 );
}

