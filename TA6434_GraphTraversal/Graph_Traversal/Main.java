/**
 *
 *  Breadth First Search (BFS) and Depth First Search (DFS) GUI program
 *  TA6434 Data Structures & Algorithm Analysis
 *
 *  Program created by:
 *  Yahya Sjahrony
 *
 *  References:
 *  1. Main Graph codes, http://algs4.cs.princeton.edu/41undirected/
 *  2. A Practical Introduction to Data Structures and Algorithm Analysis,
 *     Third Edition (C++) by Clifford A. Shaffer.
 *  3. GUI inspiration, https://www.cs.duke.edu/courses/spring12/cps100/assign/burrows/code/
 *
 *  Main/launch program.
 *
 */

public class Main {

    public static void main(String[] args){
        MainViewer sv = new MainViewer("Simple DFS/BFS Program");
        IMainProcessor proc = new SimpleMainProcessor();
        sv.setModel(proc);
    }
}
