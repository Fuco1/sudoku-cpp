/*
11. aug. 2012, public domain.
 */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

using namespace std;

struct Board {
    int board[9][9];
    bool avail[9][9][10];
    int availCount[9][9];
    bool original[9][9];
};

struct Coordinate {
    Coordinate(int x, int y) : x(x), y(y) {};
    int x;
    int y;
};

void printBoardSolution(Board& b, ostream& out) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            out << b.board[i][j];
        }
    }
    out << endl;
}

void printVector(vector<Coordinate> v) {
    for (auto i: v) {
        cout << '[' << i.x << ',' << i.y << "], ";
    }
    cout << endl;
}

void printBoardAvail(const Board& b, ostream& out) {
    for (int i = 0; i < 9; i++) {
        // print avail
        for (int j = 0; j < 9; j++) {
            if (j == 3 || j == 6) {
                out << "|";
            }

            int leftm = (11 - b.availCount[i][j]) / 2;
            int rightm = 11 - leftm - b.availCount[i][j];

            for (int k = 0; k < leftm; k++) {
                out << ' ';
            }

            for (int k = 1; k < 10; k++) {
                if (b.avail[i][j][k]) {
                    out << k;
                }
            }

            out << '(' << b.availCount[i][j] << ')';

            for (int k = 0; k < rightm; k++) {
                out << ' ';
            }
        }

        // print board
        out << "     ";
        for (int j = 0; j < 9; j++) {
            if (j == 3 || j == 6) {
                out << "|";
            }

            out << ' ' << b.board[i][j] << ' ';
        }

        out << endl;
        if (i == 2 || i == 5) {
            for (int k = 0; k < 100; k++) {
                out << '-';
            }

            out << "      ";

            for (int k = 0; k < 29; k++) {
                out << '-';
            }

            out << endl;
        }
    }
    out << endl;
}

void eliminate(Board& b, int i, int j, int p) {
    if (b.avail[i][j][p]) {
        b.avail[i][j][p] = false;
        b.availCount[i][j]--;
    }
}

void propagateCell(Board& b, int i, int j, int p = 0) {
    if (p == 0) {
        for (p = 1; p < 10; p++) {
            if (b.avail[i][j][p]) break;
        }

        b.board[i][j] = p;
    }

    // prop rows&cols
    for (int k = 0; k < 9; k++) {
        eliminate(b,i,k,p);
        eliminate(b,k,j,p);
    }

    // prop block
    for (int c = (i/3)*3; c < ((i/3)+1)*3; c++) {
        for (int r = (j/3)*3; r < ((j/3)+1)*3; r++) {
            eliminate(b,c,r,p);
        }
    }
}

void init(Board& b) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (b.board[i][j] != 0) {
                propagateCell(b,i,j,b.board[i][j]);
            }
        }
    }
}

/*
 * Initialize original cells to those which were propagated before the
 * initial guess... we can use this to prune the domains.
 *
 * TODO: replace this with tracking of domains as guesses and
 * propagation progress.
 */
void initOriginal(Board& b) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (b.board[i][j] != 0) {
                b.original[i][j] = true;
            } else {
                b.original[i][j] = false;
            }
        }
    }
}

int propagate(Board& b) {
    bool change, changespr, solved;
    do {
        // single possibility rule. Propagate as long as possible, then try other strategies.
        do {
            changespr = false;
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    if (b.availCount[i][j] == 1) {
                        changespr = true;
                        propagateCell(b,i,j);
                    }
                }
            }
        } while (changespr);

        solved = true;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (!b.board[i][j]) {
                    solved = false;
                    goto out;
                }
            }
        }

        out:
        if (solved) return 1;

        change = false;

        // Singleton rule. If there is some possible value unique for a group,
        // it has to go to that cell (i.e., there is a cell with '49' as a possibility
        // and '4' is not possible in any other cell in the group, then we can disregard
        // '9' and 4 has to go to that cell. Also known as 'hidden single'

        // save the number of cell within the block where the singleton value is.
        int singR[10];
        int singC[10];
        int singB[10];

        for (int i = 0; i < 9; i++) { // main counter (row/col/block)
            for (int k = 1; k <= 9; k++) { singR[k] = -1; singC[k] = -1; singB[k] = -1; }

            // base block
            int br = i/3;
            int bc = i%3;

            for (int t = 0; t < 9; t++) { // secondary counter (col/row/inside block+00,01,02,10,11,12,20,21,22)
                // calc the block coords
                int r = br*3 + t/3;
                int c = bc*3 + t%3;

                for (int k = 1; k <= 9; k++) { // number
                    if (b.avail[i][t][k]) { // row
                        if (singR[k] == -1) singR[k] = t;
                        else singR[k] = -2;
                    }

                    if (b.avail[t][i][k]) { // col
                        if (singC[k] == -1) singC[k] = t;
                        else singC[k] = -2;
                    }

                    if (b.avail[r][c][k]) { // block
                        if (singB[k] == -1) singB[k] = t;
                        else singB[k] = -2;
                    }
                }
            }

            // cerr << endl;

            for (int k = 1; k <= 9; k++) {
                int t = singR[k];
                if (t >= 0 && b.availCount[i][t] > 1) { // prop row
                    change = true;
                    for (int l = 1; l <= 9; l++) {
                        b.avail[i][t][l] = false;
                    }
                    b.avail[i][t][k] = true;
                    b.availCount[i][t] = 1;
                    propagateCell(b,i,t);
                }

                t = singC[k];
                if (t >= 0 && b.availCount[t][i] > 1) { // prop col
                    change = true;
                    for (int l = 1; l <= 9; l++) {
                        b.avail[t][i][l] = false;
                    }
                    b.avail[t][i][k] = true;
                    b.availCount[t][i] = 1;
                    propagateCell(b,t,i);
                }

                int r = br*3 + singB[k]/3;
                int c = bc*3 + singB[k]%3;

                if (singB[k] >= 0 && b.availCount[r][c] > 1) { // prop block
                    change = true;
                    for (int l = 1; l <= 9; l++) {
                        b.avail[r][c][l] = false;
                    }
                    b.avail[r][c][k] = true;
                    b.availCount[r][c] = 1;
                    propagateCell(b,r,c);
                }
            }
        }
    } while (change);

    return 0;
}

int findConflict(const Board& b, const vector<Coordinate>& vars) {
    printBoardAvail(b, cout);
    Coordinate last = vars.back();
    int x = last.x;
    int y = last.y;

    int rskip = 9999999;
    int skip = 0;

    for (int k = 1; k <= 9; k++) {
        bool hasMatch = false;
        cout << "Trying " << k << endl;
        // if the "affecting" cell is not a variable it is from the
        // input and therefore always rules out the currently tried
        // assignment (k).  If it is a variable, we set it a match and
        // save the "jump" distance.

        for (int l = 0; l < 9; l++) {
            cout << "    row: " << b.board[x][l] << endl;
            if (b.board[x][l] == k) {
                if (b.original[x][l]) {
                    cout << "  Constant match row [" << x << "," << l << "] for " << k << endl;
                    goto next;
                }
                skip = 0;
                // was it a variable?
                for (auto v: vars) {
                    if (++skip == 0) continue;
                    if (v.x == x && v.y == l) {
                        cout << "  Variable match row [" << v.x << "," << v.y << "] for " << k << endl;
                        hasMatch = true;
                        if (skip < rskip) {
                            rskip = skip;
                        }
                    }
                }
            }
            cout << "    col: " << b.board[x][l] << endl;
            if (b.board[l][y] == k) {
                if (b.original[l][y]) {
                    cout << "  Constant match col [" << l << "," << y << "] for " << k << endl;
                    goto next;
                }
                skip = 0;
                // was it a variable?
                for (auto v: vars) {
                    if (++skip == 0) continue;
                    if (v.x == l && v.y == y) {
                        cout << "  Variable match col [" << v.x << "," << v.y << "] for " << k << endl;
                        hasMatch = true;
                        if (skip < rskip) {
                            rskip = skip;
                        }
                    }
                }
            }
        }

        // prop block
        for (int c = (x/3)*3; c < ((x/3)+1)*3; c++) {
            for (int r = (y/3)*3; r < ((y/3)+1)*3; r++) {
                if (b.board[c][r] == k) {
                    cout << "    block: " << b.board[c][r] << endl;
                    if (b.original[c][r]) {
                        cout << "  Constant match box [" << c << "," << r << "] for " << k << endl;
                        goto next;
                    }
                    skip = 0;
                    // was it a variable?
                    for (auto v: vars) {
                        if (++skip == 0) continue;
                        if (v.x == c && v.y == r) {
                            cout << "  Variable match box [" << v.x << "," << v.y << "] for " << k << endl;
                            hasMatch = true;
                            if (skip < rskip) {
                                rskip = skip;
                            }
                        }
                    }
                }
            }
        }

        if (!hasMatch) return 9999999;

        next:
        continue;
    }

    return rskip;


    // cout << "Assigned variable: [" << x << "," << y << "]" << endl;
    // // iterate the domain of [x,y] and find the variable in history
    // // which invalidates its choice in the current step.  We can then
    // // backjump to the most recent such variable assignment.
    // for (int k = 1; k <= 9; k++) {
    //     vector<Coordinate>::const_reverse_iterator r = vars.rbegin();
    //     ++r;
    //     skip = 0;
    //     cout << "Available k is " << k << endl;
    //     for (; r != vars.rend(); ++r) {
    //         skip++;
    //         Coordinate v = *r;
    //         if (k != b.board[v.x][v.y]) {
    //             continue;
    //         }

    //         cout << "Testing conflict for [" << v.x << "," << v.y << "]{" << b.board[v.x][v.y] <<
    //             "} for " << k << endl;

    //         // if the variable was in a position which has an
    //         // effect (= constraint) on the current cell.
    //         if (v.x == x || // variable is on the same row
    //             v.y == y || // variable is on the same column
    //             (((v.x/3)*3 < x && x < ((v.x/3)+1)*3) &&
    //              ((v.y/3)*3 < y && y < ((v.y/3)+1)*3)) // same block
    //             ) {
    //             cout << "  Conflict on [" << v.x << "," << v.y << "] for " << k << endl;
    //             if (skip < rskip) {
    //                 cout << "    Setting skip to " << skip << endl;
    //                 rskip = skip;
    //                 goto next;
    //             }
    //         }
    //     }
    //     cout << "No variable affected the current variable with current value" << endl;
    //     return 9999999;

    //     next:
    //     continue;
    // }

    // return rskip;
}

int level = 0;
int guesses = 0;

int solve(Board& b, vector<Coordinate>& vars) {
    int x=10, y=10, minc = 10;
    bool solved = true;
    // find cell to guess
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (!b.board[i][j]) solved = false;
            if (b.availCount[i][j] < minc && b.availCount[i][j] > 1) {
                minc = b.availCount[i][j];
                x = i;
                y = j;
            }
            // check if there is a cell with empty domain => that
            // means this position is unsolvable!!
            // this check alone almost doubles the speed on the hartest puzzles
            if (b.availCount[i][j] == 0 && !b.board[i][j]) {
                // cout << "skip early" << endl;
                //printBoardAvail(b, cout);
                //vars.pop_back();
                return 0;
            }
        }
    }

    vars.push_back(Coordinate(x, y));
    printVector(vars);

    if (solved) {
        printBoardSolution(b,cout);
        return 1;
    }

    // this is probably useless
    if (x == 10) {
        //cout << "skip no option" << endl;
        //vars.pop_back();
        return 0;
    }

    //cout << "avail " << b.availCount[x][y] << endl;
    //printBoardAvail(b, cout);
    // guess
    for (int k = 1; k <= 9; k++) {
        if (b.avail[x][y][k]) {
            //cout << "guess " << k << endl;
            Board c;
            memcpy(&c,&b,sizeof(Board));

            c.availCount[x][y] = 1;
            for (int l = 1; l <= 9; l++) {
                c.avail[x][y][l] = false;
            }
            c.avail[x][y][k] = true;

            // if propagation alone solves the puzzle, we can skip one level of recursion
            // and print the result directly
            if (propagate(c)) {
                printBoardSolution(c,cout);
                return 1;
            }

            level++;
            int re = solve(c, vars);
            level--;
            guesses++;
            if (re) return 1;
        }
    }

    int conflict = findConflict(b, vars);
    cout << "Conflict level: " << conflict << endl;

    vars.pop_back();
    //cout << "skip" << endl;
    printVector(vars);
    return 0;
}

int main() {
    char tmp;
    string line;
    Board b;
    int clean = 0;
    while (getline(cin, line)) {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                tmp = line[i*9+j];
                if (tmp == '.') {
                    b.board[i][j] = 0;
                    b.availCount[i][j] = 9;

                    for (int k = 1; k < 10; k++) {
                        b.avail[i][j][k] = true;
                    }
                }
                else {
                    b.board[i][j] = tmp - 48;
                    b.availCount[i][j] = 0;

                    memset(b.avail[i][j],0,10*sizeof(bool));
                }
            }
        }

        init(b);
        //printBoardAvail(b, cout);
        if (propagate(b)) {
            printBoardSolution(b,cout);
            clean++;
        }
        else {
            initOriginal(b);
            guesses = 0;
            vector<Coordinate> vars;
            int re = solve(b, vars);
            if (!re) cout << "wrong" << endl;
            printVector(vars);
        }
    }
    cerr << clean << endl;
}
