/*
*  HuffmanCoding.cpp
*  A program to compress files using Huffman Compression Algorithm
*
*  Program created by:
*  1. Mohd Azi Bin Abdullah
*  2. Yahya Sjahrony
*
*  TTTA6434 ALGORITHM AND DATA STRUCTURE
*
*  Resources:
*  1. A Practical Introduction to Data Structures and Algorithm Analysis,
*     Third Edition (C++) by Clifford A. Shaffer.
*  2. A simple Huffman implementation, http://left404.com
*  3. Huffman coding, http://rosettacode.org/wiki/Huffman_coding
*
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <queue>
#include <climits> // CHAR_BIT
#include <unistd.h> // getopt
#include <ctime>

const int NUM_CHARS = (1 << CHAR_BIT) + 1; // Max number of character values: 256 bytes + FAKE_EOF
const int FAKE_EOF = NUM_CHARS - 1; // Special value to mark end of file (256)

using namespace std;

typedef map<int, string> HuffCodeMap; // Maps characters to their associated codes (bitstrings)
typedef map<int, size_t> FreqMap; // Maps characters to their associated frequencies

// Huffman node base class
class HuffNode
{
public:
    const size_t freq; // Frequency of a character
    virtual ~HuffNode() {}
protected:
    HuffNode(size_t freq) : freq(freq) {}
};

// Internal node subclass
class InternalNode : public HuffNode
{
public:
    HuffNode* left; // Left child
    HuffNode* right; // Right child
    InternalNode(HuffNode* lc, HuffNode* rc) : HuffNode(lc->freq + rc->freq), left(lc), right(rc) {}
    InternalNode() : HuffNode(0), left(NULL), right(NULL) {}
    ~InternalNode()
    {
        delete left;
        delete right;
    }
};

// Leaf node subclass
class LeafNode : public HuffNode
{
public:
    const int ch; // Character (in decimal value)
    LeafNode(size_t freq, int ch) : HuffNode(freq), ch(ch) {}
    LeafNode(int ch) : HuffNode(0), ch(ch) {}
};

// Comparator for HuffNode
struct Compare
{
    bool operator() (const HuffNode* lc, const HuffNode* rc) const {
        return(lc->freq > rc->freq);
    }
};

// Build Huffman encoding tree from a collection of frequencies
HuffNode* BuildTree(FreqMap& freqs)
{
    priority_queue<HuffNode*, vector<HuffNode*>, Compare> trees;

    for(FreqMap::const_iterator it = freqs.begin(); it != freqs.end(); it++) {
        trees.push(new LeafNode(it->second, it->first)); // The queue is sorted automatically as new entries are added.
    }

    HuffNode *tmp1, *tmp2;
    while (trees.size() > 1) {
        // Pop top two items. They are lowest frequency nodes.
        tmp1 = trees.top();
        trees.pop();
        tmp2 = trees.top();
        trees.pop();
        trees.push(new InternalNode(tmp1, tmp2)); // Insert the new internal node back into the priority queue
    }
    return(trees.top()); // This is the root node of our Huffman encoding tree
}

// Build code table
void BuildCode(const HuffNode* node, const string& prefix, HuffCodeMap& outCodes)
{
    if (const LeafNode* lf = dynamic_cast<const LeafNode*>(node)) {
        outCodes[lf->ch] = prefix;
    } else if (const InternalNode* in = dynamic_cast<const InternalNode*>(node)) {
        // Append 0 to code so far and traverse left
        string leftPrefix = prefix;
        leftPrefix += "0";
        BuildCode(in->left, leftPrefix, outCodes);
        // Append 1 to code so far and traverse right
        string rightPrefix = prefix;
        rightPrefix += "1";
        BuildCode(in->right, rightPrefix, outCodes);
    }
}

// Helper for RebuildTree
void Traverse(HuffNode* node, int& index, string const& traversal) {
    InternalNode* in = dynamic_cast<InternalNode*>(node);

    if (traversal[index+1] == 'L') { // This is the marker of the last bit of the code
        char ch = traversal[index+2]; // Get the stored character
        int val;
        if (ch == '2' && traversal[index+3] == '5' && traversal[index+4] == '6') {
            val = FAKE_EOF;
        } else {
            val = (int) ch;
        }

        if (traversal[index-1+1] == '0') {
            in->left = new LeafNode(val);
        } else if (traversal[index-1+1] == '1') {
            in->right = new LeafNode(val);
        }
        return;
    }

    if (traversal[index] == '1') {
        if (in->right == NULL){
            in->right = new InternalNode();
        }
        Traverse(in->right, ++index, traversal);

    } else if (traversal[index] == '0') {
        if (in->left == NULL){
            in->left = new InternalNode();
        }
        Traverse(in->left, ++index, traversal);
    }
}

// Rebuild Huffman tree from code table that was used to compress the file
HuffNode* RebuildTree(HuffCodeMap &outCodes)
{
    int index;
    string traversal;
    HuffNode* root = new InternalNode();
    for (HuffCodeMap::const_iterator it = outCodes.begin(); it != outCodes.end(); ++it){
        index = 0;
        traversal = it->second + "L"; // Append marker L for leaf node
        if (it->first == FAKE_EOF) {
            stringstream ss;
            ss << it->first;
            traversal += ss.str();
        } else {
            traversal += static_cast<char>(it->first);
        }
        // String traversal now should contain: code + marker L + character
        Traverse(root, index, traversal);
    }
    return(root);
}

// Display pre-order traversal
void DisplayTraversal(HuffNode* node) {
    if (!node) {
        return;
    }

    if (LeafNode* lf = dynamic_cast<LeafNode*>(node)) {
        cout << "\tLeaf Node (" << lf->ch << ")" << endl;
        return;
    }

    InternalNode* in = dynamic_cast<InternalNode*>(node);
    cout << "Internal Node " << endl;
    DisplayTraversal(in->left);
    DisplayTraversal(in->right);
}

// Convert value to string
template <typename T>
string ToStr(const T& t) {
    ostringstream os;
    os << t;
    return(os.str());
}

// Display character, frequency and code for each character
void Display(HuffCodeMap& outCodes, FreqMap& freqs){
    string s1 = "Char  ", s2 = "Frequency         ", s3 = "Code", s4 = s1 + s2 + s3;
    cout << endl << setfill('-') << setw(s4.size()) << "-" << endl;
    cout << s4 << endl;
    cout << setfill('-') << setw(s4.size()) << "-" << setfill(' ') << endl;

    for (HuffCodeMap::const_iterator it = outCodes.begin(); it != outCodes.end(); ++it) {
        cout << setw(s1.size()) << left;
        if (!isprint(it->first)) {
            switch (it->first) { // Print some white-space characters
            case 0  : cout << "'\\0'"; break;
            case 9  : cout << "'\\t'"; break;
            case 10 : cout << "'\\n'"; break;
            case 13 : cout << "'\\r'"; break;
            case 20 : cout << "' '"; break;
            case FAKE_EOF: cout << "EOF"; break;
            default : cout << it->first; // Print the character in decimal
            }
        } else {
            cout << static_cast<char>(it->first);
        }
        cout << setw(s2.size()) << left << freqs[it->first];
        cout << left << it->second;
        cout << endl;
    }
}

// Test class
class Test
{
public:
    Test(string const& filename, string const& filename2, bool decompress, bool genTable, bool useTable, bool force, bool useFreq, bool verbose);
    ~Test ();
    int Compress(void);
    int Decompress(void);

private:
    unsigned int m_bits, m_bitCount; // Buffers holding raw bits and number of bits filled
    size_t m_originalSize, m_originalSize2, m_ofileSize; // For a file with a size under 2GB we could use int but lets use size_t
    string m_fileName, m_fileName2, m_ofileName, m_ofileName2;
    fstream m_file, m_file2, m_ofile, m_ofile2;
    bool m_decompress, m_genTable, m_useTable, m_force, m_useFreq, m_verbose;
    void WriteBits(string& encoding, string& buf);
    int ReadBit(void);
    void Decode(const HuffNode* node, int& ch);
    void ReadTable(fstream& fs, FreqMap& freqs, HuffCodeMap& codes);
};

// Constructor
Test::Test(string const& fileName, string const& fileName2, bool decompress, bool genTable, bool useTable, bool force, bool useFreq, bool verbose) : m_bits(0), m_bitCount(0)
{
    m_fileName = fileName;
    m_decompress = decompress;
    m_genTable = genTable;
    m_useTable = useTable;
    m_force = force;
    m_useFreq = useFreq;
    m_verbose = verbose;
    string ext = "z";

    if (m_decompress) {
        m_ofileName = fileName;
        // Remove extension .z if any
        if (m_ofileName.substr(m_ofileName.find_last_of(".") + 1) == ext) {
            m_ofileName.erase(m_ofileName.find_last_of("." + ext) - ext.size(), string::npos);
        } else {
            ext = "";
            if(m_ofileName.find_last_of(".") != string::npos) {
                ext = m_ofileName.substr(m_ofileName.find_last_of(".") + 1);
            }
            !ext.empty()?m_ofileName += "." + ext : m_ofileName += ".txt";
        }

    } else {
        m_ofileName = fileName + "." + ext;
    }

    if(ifstream(m_ofileName.c_str())) {
        cerr << "ERROR: Output file already exists! \""<< m_ofileName.c_str() << "\"" <<  endl;
        exit(1);
    }

    if (m_genTable) {
        m_ofileName2 = fileName + ".lst";

    } else if (m_useTable) {
        m_fileName2 = fileName2;
    }

    // Open the file with the ios::ate flag (ate: seek to the end of stream immediately after open)
    m_file.open(m_fileName.c_str(), ios::in | ios::ate | ios::binary);
    if (m_file.fail()) {
        cerr << "ERROR: Cannot open input file \"" << m_fileName << "\"" << endl;
        exit(1);
    }

    m_originalSize = m_file.tellg(); // Tell the location of get pointer, i.e find the size

    m_file.clear(); // Clear the eof flag
    m_file.seekg(0); // Set the get pointer at the beginning of the file
    if (m_originalSize < 1){
        cerr << "ERROR: Got empty file! \"" << m_fileName << "\"" << endl;
        exit(1);
    }

    if (m_useTable) {
        m_file2.open(m_fileName2.c_str(), ios::in | ios::ate | ios::binary);
        if (m_file2.fail()) {
            cerr << "ERROR: Cannot open input file \"" << m_fileName2 << "\"" << endl;
            exit(1);
        }
        m_originalSize2 = m_file2.tellg();
        m_file2.clear();
        m_file2.seekg(0);
        if (m_originalSize2 < 1){
            cerr << "ERROR: Got empty file \"" << m_fileName2 << "\"" << endl;
            exit(1);
        }
    }
}

// Destructor
Test::~Test ()
{
    m_file.close();
    m_ofile.close();
    if (m_genTable) {
        m_ofile2.close();
    } else if (m_useTable) {
        m_file2.close();
    }
}

// Write the given bit encoding to temporary buffer
void Test::WriteBits(string& encoding, string& buf)
{
    for (string::const_iterator it = encoding.begin(); it != encoding.end(); ++it) {
        m_bits = 2*m_bits + *it - '0'; // Push bits on from the right
        m_bitCount++;

        if (m_bitCount == CHAR_BIT) {
            buf.push_back(m_bits);
            m_bits = 0;
            m_bitCount = 0;
        }
    }
}

// Read a single bit from the input file
int Test::ReadBit(void)
{
    unsigned int nextBit;
    if (m_bitCount == 0) {
        m_bits = m_file.get();
        m_bitCount = (1 << (CHAR_BIT - 1));
    }

    nextBit = m_bits / m_bitCount;
    m_bits %= m_bitCount;
    m_bitCount /= 2;

    return(nextBit);
}

// Decode
void Test::Decode(const HuffNode* node, int& ch) {
    while (const InternalNode* in = dynamic_cast<const InternalNode*>(node)) {
        ReadBit() ? node = in->right : node = in->left;
    }
    ch = dynamic_cast<const LeafNode*>(node)->ch;
}

//
// Read table
// FILE STRUCTURE:
// [Total characters]\n
// [Character (in decimal)] [Frequency/Code]\n
// [Data]FAKE_EOF
//
void Test::ReadTable(fstream& fs, FreqMap& freqs, HuffCodeMap& codes){ //TODO: sanity check for code?
    int totalChars, startPos;
    int err = 0;
    string line;

    if (!(fs >> totalChars)) { // Get total characters
        err = 1;
        cerr << "ERROR: Malformed file header (" << err << ")" << endl;
        exit(1);
    }

    if (totalChars > NUM_CHARS) {
        err = 2;
        cerr << "ERROR: Malformed file header (" << err << ")" << endl;
        exit(1);
    }

    startPos = fs.tellg(); // Tell the current position of get stream pointer

    startPos++; // Skip new line

    fs.seekg(startPos); // Put the get pointer to the position startPos

    for(int i = 0; i < totalChars; i++) {
        getline(fs, line);
        int a;
        string b;

        istringstream iss(line);
        if (!(iss >> a >> b)) {
            err = 3;
            cerr << "ERROR: Malformed file header (" << err << ")" << endl;
            exit(1);
        }

        if (m_useFreq){
            size_t f;
            stringstream ss(b);
            if ( !(ss >> f) ) {
                err = 4;
                cerr << "ERROR: Malformed file header (" << err << ")" << endl;
                exit(1);
            }
            freqs[a] = f;
        } else {
            codes[a] = b;
        }
    }
    if (m_useFreq){
        if (freqs.find(FAKE_EOF) == freqs.end()) {
            err = 5;
            cerr << "ERROR: Malformed file header (" << err << ")" << endl;
            exit(1);
        }
    } else {
        if (codes.find(FAKE_EOF) == codes.end()) {
            err = 5;
            cerr << "ERROR: Malformed file header (" << err << ")" << endl;
            exit(1);
        }
    }
}

// Decompress
int Test::Decompress(void)
{
    FreqMap freqs;
    HuffCodeMap codes;

    m_useTable ? ReadTable(m_file2, freqs, codes) : ReadTable(m_file, freqs, codes);

    int startPos;
    startPos = m_file.tellg(); // Tell the current position of get stream pointer so far
    m_file.seekg(startPos); // Put the get pointer to the position startPos

    HuffNode* root;

    if (m_useFreq) {
        root = BuildTree(freqs);
        BuildCode(root, string(), codes);
    } else {
        root = RebuildTree(codes);
    }

    string buf;
    int ch;
    while (m_file.good()) {
        Decode(root, ch);
        if (ch == FAKE_EOF){
            break;
        }
        buf.push_back(static_cast<char>(ch));
    }

    size_t tableSize;
    if (m_useTable)
       tableSize = m_file2.tellg();
    else {
       tableSize = (size_t) startPos;
    }
    m_ofileSize = buf.size();

    if (m_verbose) {
        cout << endl
        << "Original compressed size: " << m_originalSize << " bytes\n"
        << "Table size: " << tableSize << " bytes\n"
        << "Original size: " << m_ofileSize << " bytes\n"
        << endl;
    }

    m_ofile.open(m_ofileName.c_str(), ios::out | ios::ate | ios::binary);
    if (m_ofile.fail()) {
        cerr << "ERROR: Cannot open output file \"" << m_ofileName << "\"" << endl;
        exit(1);
    }

    m_ofile << buf;
    m_ofile.close();

    delete root;
    return(0);
}

// Compress
int Test::Compress(void)
{
    unsigned char* in = new unsigned char[m_originalSize];

    m_file.read((char*)in, m_originalSize);
    if (!m_file) {
        cerr << "ERROR: Only " << m_file.gcount() << " could be read." << endl;
        exit(1);
    }

    FreqMap freqs;
    HuffCodeMap codes;

    // Build frequency table
    for(unsigned int k = 0; k < m_originalSize; ++k) {
        freqs[static_cast<unsigned char>(in[k])]++;
    }

    freqs[FAKE_EOF] = 1; // Add FAKE_EOF
    HuffNode* root = BuildTree(freqs);
    BuildCode(root, string(), codes);
    //DisplayTraversal(root);
    delete root;

    if (m_verbose) {
        Display(codes, freqs);
    }

    // Write file header needed for the decompression process
    string table;
    table+=ToStr(codes.size()); // Write total unique characters
    table+='\n';
    for (HuffCodeMap::const_iterator it = codes.begin(); it != codes.end(); ++it) {
        table+=ToStr(it->first); // Write the character
        table+=' ';
        if (m_useFreq) {
            table+=ToStr(freqs[it->first]); // Write the frequency
        } else {
            table+=it->second; // Write code
        }
        table+='\n';
    }

    // Read the input a second time. For each character read,
    // write the encoding of the character (obtained from the
    // map of encodings) to the compressed file.
    size_t encodedBits = 0;
    string code, buf;
    for(unsigned int k = 0; k < m_originalSize; ++k) {
        code = codes[static_cast<unsigned char>(in[k])];
        encodedBits+=code.size();
        WriteBits(code, buf);
    }
    delete[] in;

    // Write the encoding for FAKE_EOF
    code = codes[FAKE_EOF];
    encodedBits+=code.size();
    WriteBits(code, buf);

    // Write an extra 8 blank bits to flush the output buffer
    code = "0000000";
    //encodedBits+=code.size();
    WriteBits(code, buf);

    // Calculate some stats
    size_t tableSize = table.size();
    size_t encodedSize = buf.size();
    size_t total = tableSize + encodedSize;
    size_t originalBits = 8 * sizeof(char) * m_originalSize;

    if (total < m_originalSize) {
        cout << "\n\t"
        << "Compression: "<< total << "/"<< m_originalSize <<" bytes ("
        << setprecision(4) << (double)total/(double)m_originalSize*100 <<" %)"
        << endl;
    } else {
        if (!m_force) { //XXX???
            cout << "\n"
            << "WARNING: It seems output size is bigger than the original.\n"
            << "Use -f to force compression." << endl;
            return(-1);
        }
    }

    if (m_verbose) {
        cout
        << "\n"
        << "Original size: " << m_originalSize << " bytes (" << originalBits << " bits)\n"
        << "Table size: " << tableSize << " bytes\n"
        << "Encoded size without table: " << encodedSize << " bytes (real: " << encodedBits << " bits)\n"
        << "Total size: " << total << " bytes\n"
        << "Compression ratio: " << total << "/"<< m_originalSize << " ("
        << setprecision(4) << (double) total / (double) m_originalSize << ")\n"
        << endl;
    }

    m_ofile.open(m_ofileName.c_str(), ios::out | ios::ate | ios::binary);
    if (m_ofile.fail()) {
        cerr << "ERROR: Cannot open output file \"" << m_ofileName << "\"" << endl;
        exit(1);
    }

    if (m_genTable) {
        m_ofile2.open(m_ofileName2.c_str(), ios::out | ios::ate | ios::binary);
        if (m_ofile2.fail()) {
            cerr << "ERROR: Cannot open output file \"" << m_ofileName2 << "\"" << endl;
            exit(1);
        }
    }

    // Now do actual writing
    m_genTable?m_ofile2 << table : m_ofile << table;

    m_ofile << buf;
    m_ofile.close();

    if (m_genTable) {
        m_ofile2.close();
    }

    return(0);
}

// Print usage
void printUsage(const string name){
    cout
    << "Usage: "<< name <<" [OPTIONS] [FILE]\n"
    << "\n"
    << "Compress FILE using Huffman Compression Algorithm\n"
    << "\n"
    << "Options:\n"
    << "  -d    Decompress\n"
    << "  -g    Generate code tree\n"
    << "  -t <FILE>    Use this code tree for decompression\n"
    << "  -f    Force\n"
    << "  -1    Use Character-frequency table in header (default:\n"
    << "        use character-code table).\n"
    << "  -h    Print this help\n"
    << "  -v    Verbose mode\n"
    << "\n"
    << "" << endl;
}

// Main program
int main(int argc, char* argv[])
{
    clock_t start = clock();
    int op;
    int dflag = 0, gflag = 0, tflag = 0, fflag = 0, oflag = 0, vflag = 0;
    string fileName, fileName2;

    // Read the parameters
    while ((op = getopt(argc, argv, "hdgf1vt:")) != -1) {
        switch (op) {
        case 'h': printUsage(argv[0]); exit(0);
        case 'd': dflag++; break;
        case 'g': gflag++; break;
        case 't': fileName2 = optarg; tflag++; break;
        case 'f': fflag++; break;
        case '1': oflag++; break;
        case 'v': vflag++; break;
        default : goto usage;
        }
    }

    if (argc - optind != 1) { //extern int optind
        if (argc == 1) {
            cout << "Enter a file name: ";
            cin >> fileName;
            vflag++;
        }
    } else {
        fileName = argv[optind];
    }

    if ( (gflag && dflag) || (gflag && tflag) ) {
        cerr << argv[0] << ": Cannot use option -g with -d or -t" << endl;
        goto usage;
    } else {
        if (tflag) {
            dflag++;
        }
    }

    if ( argc != 1 && (fileName.empty()) ) {
        cerr << argv[0] << ": No input file specified" << endl;
usage:
	if (argc > 1) {
            cerr << "Try `" << argv[0] << " -h' for more information." << endl;
        }
        exit(1);
    }

    if (vflag) {
        cout << endl
        << "Input file name: "<< fileName << "\n"
        << "Decompress flag: "<< dflag << "\n"
        << "Gen table flag: "<< gflag << "\n"
        << "Input table flag: "<< tflag << ", input table: "<< fileName2 << "\n"
        << "Force flag: "<< fflag << "\n"
        << "Use character-frequency flag: "<< oflag << endl;
    }

    Test test(fileName, fileName2, dflag, gflag, tflag, fflag, oflag, vflag);

    !dflag ? test.Compress() : test.Decompress();

    clock_t end = clock();
    if (vflag) {
        cout << "Done in " << (double)((double)end - (double)start)/(double)CLOCKS_PER_SEC << " seconds." << endl;
    }
    return(0);
}
