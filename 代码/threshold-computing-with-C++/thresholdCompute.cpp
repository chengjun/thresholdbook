#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <time.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>

#define SOCIAL_GRAPH_FILE_NAME "../SocialGraphAll.new"
#define RETWEET_LIST_FILE_NAME "../reformatted_retweet.txt"
#define THRESHOLD_OUT_FILE_NAME "../threshold.txt"

#define MAX_NODE 65767000
#define MAX_EDGE 906336000

using namespace std;

vector<int> Followee[MAX_NODE];
set<int> allNodeSet;
int nodeNumber=0;
int edgeNumber=0;

string socialGraphInputFileName = SOCIAL_GRAPH_FILE_NAME;
string retweetListInputFileName = RETWEET_LIST_FILE_NAME;
string thresholdOutputFileName = THRESHOLD_OUT_FILE_NAME;


void printProgressBar(int x, int n, int r, int w=50)
{
    if (x>n) x=n;
    // Only update r times.
    if ( x % (n/r) != 0 ) return;

    // Calculuate the ratio of complete-to-incomplete.
    float ratio = x/(float)n;
    int   c     = ratio * w;

    // Show the percentage complete.
    printf("%3d%% [", (int)(ratio*100) );

    // Show the load bar.
    for (int x=0; x<c; x++)
       printf("=");

    for (int x=c; x<w; x++)
       printf(" ");

    // ANSI Control codes to go back to the
    // previous line and clear it.
    printf("]\n\033[F\033[J");
}

bool ConstructNetwork()
{
    FILE* fr;
    fr = fopen(socialGraphInputFileName.c_str(), "r");
    if (fr==NULL)
    {
        cout << "Cannot open " << socialGraphInputFileName << endl;
        return false;
    }
    cout << "\nLoading followee networks from \n" << socialGraphInputFileName << endl;
    char s[300];
    int currentLine = 0;
    // a --> b
    int a = 0;
    int b = 0;
    while(fgets(s, 300, fr) != NULL)
    {
        currentLine++;
        printProgressBar(currentLine, MAX_EDGE, 100);
        ///format:
        // follower followee
        sscanf(s,"%d %d", &a, &b);
        allNodeSet.insert(a);
        allNodeSet.insert(b);
        Followee[a].push_back(b);
    }
    fclose(fr);
    nodeNumber = allNodeSet.size();
    edgeNumber =currentLine;
    printf("Network constructed.\n%d users and %d edges.\n", nodeNumber, edgeNumber);
    return true;
}

int main()
{
    ConstructNetwork();
    /// scan the retweet list
    FILE* fr;
    FILE* fw;
    fr = fopen(retweetListInputFileName.c_str(), "r");
    fw = fopen(thresholdOutputFileName.c_str(), "w");
    fprintf(fw, "rts.mid\trt.user\tmid\tfollowee.all\tfollowee.retweeted\tthreshold\n");
    if (fr==NULL)
    {
        cout << "Cannot open " << retweetListInputFileName << endl;
        return false;
    }
    cout << "\nScanning retweet list from \n" << retweetListInputFileName << endl;
    cout << "....." << endl;
    char s[300];
    char timeStr[20];
    char dateStr[20];
    char rts_mid[32];
    int u;
    char orig_mid[32];
    string currentRtsMid = "";
    set<int> retweetedUserSet;

    int currentLine = 0;
    while(fgets(s, 300, fr) != NULL)
    {
        currentLine++;
        if (currentLine==1) continue;   // skip header line
        sscanf(s, "%s\t%d\t%s %s\t%s", rts_mid, &u, dateStr, timeStr, orig_mid);
        // reconvert to string
        string rts_mid_str = rts_mid;
        string orig_mid_str = orig_mid;
        if (rts_mid!=currentRtsMid)
        {
            currentRtsMid = rts_mid;
            retweetedUserSet.clear();
        }
        retweetedUserSet.insert(u);
        int followeeAll = 0;
        int followeeRetweeted = 0;
        for (int i=0; i<Followee[u].size(); i++)
        {
            followeeAll++;
            // v is a followee of u
            int v = Followee[u][i];
            // v is found to be retweeted the message earlier
            if (retweetedUserSet.find(v)!=retweetedUserSet.end()) followeeRetweeted++;
        }
        float threshold = 0.0;
        if (followeeAll!=0)
        {
            threshold = (float) followeeRetweeted / (float) followeeAll;
            //rts.mid   rt.user orig.mid followee.all    followee.retweeted  threshold
            fprintf(fw, "%s\t%d\t%s\t%d\t%d\t%.6f\n", rts_mid_str.c_str(), u, orig_mid_str.c_str(),
                                                    followeeAll, followeeRetweeted, threshold);
        }
        else
        {
            // threshold = NA
            // if no followee at all
            fprintf(fw, "%s\t%d\t%s\t%d\t%d\tNA\n", rts_mid_str.c_str(), u, orig_mid_str.c_str(), followeeAll, followeeRetweeted);
        }
    }
    fclose(fr);
    fclose(fw);
    printf("Retweets scanned by reading over %d lines.", currentLine-1);
    cout << "File written to\n" << thresholdOutputFileName << endl;
    return 0 ;
}
