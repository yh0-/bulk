import java.io.IOException;

public class SimpleMainProcessor implements IMainProcessor {

    private MainViewer myViewer;
    private Graph G;

    public void setViewer(MainViewer viewer) {
        myViewer = viewer;
    }

    private void showString(String s){
        myViewer.update(s);
    }

    public int preProcess(In in) throws IOException {
        //throw new IOException("preprocess not implemented");
        //showString("preprocess is running\n");
        G = new Graph(in);
        return 0;
    }

    public int DFS(In in, int s) throws IOException {
        showString(" * Depth First Paths: (DFS Implemention)\n");

        DepthFirstPaths dfs = new DepthFirstPaths(G, s);

        for (int v = 0; v < G.V(); v++) {
            if (dfs.hasPathTo(v)) {
                showString(s + " to " + v + ":   ");
                for (int x : dfs.pathTo(v)) {
                    if (x == s) {
                        showString(" " + x);
                    } else {
                        showString("-" + x);
                    }
                }
                showString("\n");
            }

            else {
                showString(s + " to " + v + ":  not connected\n");
            }

        }
        showString("\n");
        return 0;
    }

    public int BFS(In in, int s) throws IOException {
        showString(" * Breadth First Paths: (BFS Implemention)\n");

        BreadthFirstPaths bfs = new BreadthFirstPaths(G, s);

        for (int v = 0; v < G.V(); v++) {
            if (bfs.hasPathTo(v)) {
                showString(s + " to " + v + " ("+bfs.distTo(v)+"):   ");
                for (int x : bfs.pathTo(v)) {
                    if (x == s) {
                        showString(" " + x);
                    } else {
                        showString("-" + x);
                    }
                }
                showString("\n");
            }

            else {
                showString(s + " to " + v + " (-):  not connected\n");
            }
        }
        showString("\n");
        return 0;
    }
}
