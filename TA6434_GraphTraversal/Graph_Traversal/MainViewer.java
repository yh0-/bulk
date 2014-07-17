import javax.swing.*;

import java.awt.BorderLayout;
import java.awt.event.*;
import java.util.*;
import java.io.*;

/**
 * The GUI/View for DFS/BFS program.
 *
 */
@SuppressWarnings("serial")
public class MainViewer extends JFrame {

    private boolean dfs, about;
    protected JTextArea myOutput;
    protected IMainProcessor myModel;
    protected String myTitle;
    protected JTextField myMessage;
    protected File myFile;
    private boolean debug;

    protected static JFileChooser ourChooser =
        new JFileChooser(System.getProperties().getProperty("user.dir"));

    public MainViewer(String title) {
        setDefaultCloseOperation(EXIT_ON_CLOSE);

        JPanel panel = (JPanel) getContentPane();
        panel.setLayout(new BorderLayout());
        setTitle(title);
        myTitle = title;
        debug = false;

        panel.add(makeOutput(), BorderLayout.CENTER);
        panel.add(makeMessage(), BorderLayout.SOUTH);
        makeMenus();

        pack();
        setSize(400, 400);
        setVisible(true);
    }

    /**
     * Associates this view with the given model. The GUI/View will
     * attach itself to the model so that communication between the view
     * and the model as well as <em>vice versa</em> is supported.
     * @param model is the model for this view
     */
    public void setModel(IMainProcessor model) {
        myModel = model;
        myModel.setViewer(this);
    }

    protected JPanel makeMessage() {
        JPanel p = new JPanel(new BorderLayout());
        myMessage = new JTextField(30);
        p.setBorder(BorderFactory.createTitledBorder("message"));
        p.add(myMessage, BorderLayout.CENTER);
        return p;
    }

    protected JPanel makeOutput() {
        JPanel p = new JPanel(new BorderLayout());
        myOutput = new JTextArea(10,40);
        p.setBorder(BorderFactory.createTitledBorder("output"));
        p.add(new JScrollPane(myOutput), BorderLayout.CENTER);
        return p;
    }

    protected JMenu makeFileMenu() {
        JMenu fileMenu = new JMenu("File");

        fileMenu.add(new AbstractAction("Clear") {
            public void actionPerformed(ActionEvent ev) {
                clear();
                about = false;
            }
        });

        fileMenu.add(new AbstractAction("DFS") {
            public void actionPerformed(ActionEvent ev) {
                dfs = true;
                doTraverse();
            }
        });

        fileMenu.add(new AbstractAction("BFS") {
            public void actionPerformed(ActionEvent ev) {
                dfs = false;
                doTraverse();
            }
        });

        fileMenu.add(new AbstractAction("Quit") {
            public void actionPerformed(ActionEvent ev) {
                System.exit(0);
            }
        });

        return fileMenu;
    }

    protected JMenu makeOptionsMenu() {
        JMenu optionMenu = new JMenu("Option");


        JCheckBoxMenuItem debugOption = new JCheckBoxMenuItem(new AbstractAction(
                "Debug") {
            public void actionPerformed(ActionEvent ev) {
                debug = !debug;
            }
        });

        optionMenu.add(debugOption);

        optionMenu.add(new AbstractAction("About") {
            public void actionPerformed(ActionEvent ev) {
                if (!about) {
                    clear();
                    update(" * Simple program to find shortest paths (number of edges) \n");
                    update("   from a source vertex to every other vertex \n");
                    update("   in an undirected graph. \n\n");
                    update(" * This implementation uses Depth-first search (DFS) \n");
                    update("   and Breadth-first search (BFS) algorithms. \n\n");
                about = true;
                }
            }
        });

        return optionMenu;
    }

    protected void makeMenus() {
        JMenuBar bar = new JMenuBar();
        bar.add(makeFileMenu());
        bar.add(makeOptionsMenu());
        setJMenuBar(bar);
    }

    /**
     * Clear the text area, e.g., for a new message.
     */
    public void clear(){
        showMessage("");
        myOutput.setText("");
    }

    /**
     * To be called by model/client code to display strings in the GUI. Displays string
     * on a single line. Call multiple times with no interleaved clear to show several
     * strings.
     * @param s is string to be displayed
     */
    public void update(String s) {
        myOutput.append(s+"");
    }

    /**
     * Display a text message in the view (e.g., in the small text area
     * at the bottom of the GUI), thus a modeless message the user can ignore.
     * @param s is the message displayed
     */
    public void showMessage(String s) {
        myMessage.setText(s);
    }

    /**
     * Show a modal-dialog indicating an error; the user must dismiss the
     * displayed dialog.
     * @param s is the error-message displayed
     */
    public void showError(String s) {
        JOptionPane.showMessageDialog(this, s, "Error",
                JOptionPane.INFORMATION_MESSAGE);
    }

    protected File doRead() {
        int retval = ourChooser.showOpenDialog(null);
        if (retval != JFileChooser.APPROVE_OPTION) {
            return null;
        }
        showMessage("reading/initializing");
        myFile = ourChooser.getSelectedFile();
        MainViewer.this.showMessage("reading "+ myFile +" ");

        try {
            final In in = new In(myFile);
            int saved = myModel.preProcess(in);
        } catch (Exception e) {
            MainViewer.this.showError("Caught exception!\n "+e);
            //e.printStackTrace();
            return null;
        }
        File ret = myFile;
        myFile = null;
        return ret;
    }

    private void doTraverse() {

        myFile = doRead();
        if (myFile == null){
            return;
        }

        final In in = new In(myFile);

        String name = myFile.getName();
        showMessage("processing "+name);

        String inputSource = JOptionPane.showInputDialog(this,
                "Enter source vertex ", "0");

        int sourceVertex;

        try {
            sourceVertex = Integer.parseInt(inputSource);
        } catch (NumberFormatException e) {
            MainViewer.this.showError("Caught exception!\n "+e);
            return;
        }

        //MainViewer.this.showMessage("sourceVertex: "+ sourceVertex +" ");

        String path = null;
        try {
            path = myFile.getCanonicalPath();
        } catch (IOException e) {
            showError("trouble with file canonicalizing");
            return;
        }

        try {
            if (dfs) {
                myModel.DFS(in, sourceVertex);
            } else {
                myModel.BFS(in, sourceVertex);
            }
        } catch (Exception e) {
            MainViewer.this.showError("Caught exception!\n "+e);
            //e.printStackTrace();
            return;
        }

        myFile = null;
    }
}

