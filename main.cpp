#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

class chainCode {
public:
    struct point {
        int row;
        int col;
    };
    struct CCproperty {
        int label;
        int numpixels;
        int minRow, minCol, maxRow, maxCol; 
    };
    int numCC;
    CCproperty CC; 
    int numRows, numCols, minVal, maxVal;
    int** imageAry; 
    int** boundaryAry; 
    int** CCAry; 
    point coordOffset[8]; 
    point neighborCoord[8]; 
    int zeroTable[8] = {6, 0, 0, 2, 2, 4, 4, 6}; 
    point startP, currentP, nextP;
    int lastQ, nextQ; 
    int PchainDir; 

    chainCode(ifstream &labelfile, ifstream &propertyfile) {
        std::string line, line2, line3;

        getline(labelfile, line);
        getline(propertyfile, line2);
        getline(propertyfile, line3);

        stringstream ssin(line);
        stringstream ssin2(line2);
        stringstream ssin3(line3);

        ssin >> numRows >> numCols >> minVal >> maxVal;
        ssin2 >> numRows >> numCols >> minVal >> maxVal;
        ssin3 >> numCC;
        
        imageAry = new int *[numRows + 2]; 
        boundaryAry = new int *[numRows + 2];
        CCAry = new int *[numRows + 2]; 

        for (int i=0; i<numRows+2; i++) {
            imageAry[i] = new int [numCols + 2]; 
            boundaryAry[i] = new int [numCols + 2];
            CCAry[i] = new int [numCols + 2]; 
        }

        setCoordOffset();
    }

    ~chainCode() {
        for (int i=0; i<numRows+2; i++) {
            delete[] imageAry[i];
            delete[] boundaryAry[i];
            delete[] CCAry[i];
        }
        delete[] imageAry;
        delete[] boundaryAry;
        delete CCAry;
    }

    void setCoordOffset() {
        coordOffset[0].row = 0;
        coordOffset[0].col = 1;
        coordOffset[1].row = -1;
        coordOffset[1].col = 1;
        coordOffset[2].row = -1;
        coordOffset[2].col = 0;
        coordOffset[3].row = -1;
        coordOffset[3].col = -1;
        coordOffset[4].row = 0;
        coordOffset[4].col = -1;
        coordOffset[5].row = 1;
        coordOffset[5].col = -1;
        coordOffset[6].row = 1;
        coordOffset[6].col = 0;
        coordOffset[7].row = 1;
        coordOffset[7].col = 1;
    }

    void zeroFramedAry() {
        for (int i=0; i<numRows+2; i++) {
            for (int j=0; j<numCols+2; j++) {
                imageAry[i][j] = 0;
                boundaryAry[i][j] = 0;
                CCAry[i][j] = 0;
            }
        }
    }

    void loadImage(ifstream &inFile) {
        int row = 1;
        std::string line;
        while (std::getline(inFile, line)) {
            stringstream ssin(line);
            int number;
            for (int i = 1; i < numCols+1; i++) {
                ssin >> number;
                imageAry[row][i] = number;
            }
            row++;
        }
    }

    void clearCCAry() {
        for (int i=0; i<numRows+2; i++) {
            for (int j=0; j<numCols+2; j++) {
                CCAry[i][j] = 0;
            }
        }
    }

    void loadCCAry() { 
        for (int i=0; i<numRows+2; i++) {
            for (int j=0; j<numCols+2; j++) {
                if (imageAry[i][j] == CC.label) {
                    CCAry[i][j] = CC.label;
                } else
                    CCAry[i][j] = 0;
            }
        }
    }

    void getChainCode(ofstream &ChainCodeFile) {
        int label = CC.label;
        for (int i=0; i<numRows+2; i++) {
            for (int j=0; j<numCols+2; j++) {
                if (CCAry[i][j] == label) { 
                    ChainCodeFile << endl << label << " " << i << " " << j << endl;
                    startP.row = i;
                    startP.col = j;
                    currentP.row = i;
                    currentP.col = j;
                    lastQ = 4;
                    boundaryAry[i][j] = label;
                    goto Continue;
                } 
            }
        }

        Continue:
            bool end = false;
            while (!end) {  
                nextQ = (lastQ + 1) % 8;
                PchainDir = findNextP(currentP);
                nextP.row = neighborCoord[PchainDir].row;
                nextP.col = neighborCoord[PchainDir].col;
                CCAry[currentP.row][currentP.col] = -CCAry[currentP.row][currentP.col]; 
                ChainCodeFile << PchainDir << " ";

                boundaryAry[nextP.row][nextP.col] = label;

                if (PchainDir == 0)
                    lastQ = zeroTable[7];
                else    
                    lastQ = zeroTable[PchainDir-1];
                currentP.row = nextP.row;
                currentP.col = nextP.col;
                if (currentP.row == startP.row && currentP.col == startP.col) {
                    end = true; 
                }
            } 
    }

    void loadNeighborsCoord(point current) { 
        for (int i=0; i<8; i++) {
            neighborCoord[i].row = coordOffset[i].row + current.row;
            neighborCoord[i].col = coordOffset[i].col + current.col;
        }
    }

    int findNextP(point currentP) {
        loadNeighborsCoord(currentP);
        int index = lastQ;
        bool found = false;

        while (found != true) {
            int iRow = neighborCoord[index].row;
            int jCol = neighborCoord[index].col;
            if (imageAry[iRow][jCol] == CC.label) {
                PchainDir = index; 
                found = true;
            }
            index = (index+1) % 8;
        }

        return PchainDir; 
    }

    void reformatPrettyPrint (int** array, ofstream &outFile) {
        string str = to_string(maxVal);
        int width = str.length();
        int r = 0;
    
        while (r < numRows+2) {
        int c = 0;
        while (c < numCols+2) {
            if (array[r][c] == 0) {
            outFile << ". ";
            } else {
            outFile << array[r][c] << " ";
            }
            str = to_string(array[r][c]);
            int ww = str.length();
    
            while (ww <= width) {
                outFile << " ";
                ww++;
            }
            c++;
        }
        outFile << endl;
        r++;
        }
    
        outFile << endl;
    }

    void printBoundaryFile(ofstream &BoundaryFile) {
        BoundaryFile << numRows << " " << numCols << " " << minVal << " " << maxVal << endl;
        for (int i=1; i<numRows+1; i++) {
            for (int j=1; j<numCols+1; j++) {
            BoundaryFile << boundaryAry[i][j] << " ";
            }
        BoundaryFile << endl;
        }
    }
};

int main (int argc, char *argv[]) {
    ifstream labelFile(argv[1]);
    ifstream propFile(argv[2]);

    chainCode c(labelFile, propFile);
    c.loadImage(labelFile);

    std::string toReplace(".txt");

    std::string ChainCodeFileName(argv[1]);
    std::string BoundaryFileName(argv[2]);

    size_t pos1 = ChainCodeFileName.find(toReplace);
    size_t pos2 = BoundaryFileName.find(toReplace);

    ChainCodeFileName.replace(pos1, toReplace.length(), "_chainCode.txt");
    BoundaryFileName.replace(pos2, toReplace.length(), "_Boundary.txt");

    ofstream ChainCodeFile(ChainCodeFileName);
    ofstream BoundaryFile(BoundaryFileName);

    ChainCodeFile << c.numRows << " " << c.numCols << " " << c.minVal << " " << c.maxVal << endl;
    ChainCodeFile << c.numCC;

    while (!propFile.eof()) {
        std::string line1, line2, line3, line4;

        getline(propFile, line1);
        getline(propFile, line2);
        getline(propFile, line3);
        getline(propFile, line4);

        stringstream ssin1(line1);
        stringstream ssin2(line2);
        stringstream ssin3(line3);
        stringstream ssin4(line4);

        ssin1 >> c.CC.label;
        ssin2 >> c.CC.numpixels;
        ssin3 >> c.CC.minRow >> c.CC.minCol;
        ssin4 >> c.CC.maxRow >> c.CC.maxCol;

        c.clearCCAry();
        c.loadCCAry(); 
        c.getChainCode(ChainCodeFile);
    }

    c.printBoundaryFile(BoundaryFile);

    labelFile.close();
    propFile.close();
    ChainCodeFile.close();
    BoundaryFile.close();
}