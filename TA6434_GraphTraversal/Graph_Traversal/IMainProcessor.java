/**
 * The interface for the model that can be attached
 * to a MainViewer.
 *
 */

import java.io.IOException;

public interface IMainProcessor {

    public void setViewer(MainViewer viewer);
    public int preProcess(In in) throws IOException;
    public int DFS(In in, int s) throws IOException;
    public int BFS(In in, int s) throws IOException;
}
