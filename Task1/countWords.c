#include <stdio.h>
#include <stdbool.h>
#include "dataStructures.h"
#include "countWords.h"

void count(Chunk *in, TempResults *out)
{
    int display;

    int a = 0;
    bool aFlag = true;
    int e = 0;
    bool eFlag = true;
    int i = 0;
    bool iFlag = true;
    int o = 0;
    bool oFlag = true;
    int u = 0;
    bool uFlag = true;
    int c = 0;
    bool cFlag = true;
    int w = 0;
    bool go = false;
    int prev = 0;
    int newWord[] = {166,147,157,156,32,46,44,58,59,45,63,33,34,40,41,91,9,10,13,194,187,171};
    int arrLen = sizeof newWord / sizeof newWord[0];
    bool inWord = false;

    for(int p = 0 ; p < in->numBytes ; p++ )
    {
        // reading file
        display = in->textChunk[p];

        bool special = false;
        bool pastInWord = inWord;
        // UTF-8  -->  E280**
        if( display == 226){
            p++;
            display = in->textChunk[p];
            special = true;
            p++;
            display = in->textChunk[p];
        }
        // end of file indicator
        if ( !(p<in->numBytes) ) break;
        
        //printf("%i\n", display);
        if( prev != 195 ){
            bool tmpInWord = true;
            if(!inWord) if ( display == 95 ) tmpInWord = true;
            if(inWord) if ( display == 39 || display == 95 || ( special && display == 152 ) || ( special && display == 153 )) tmpInWord = true;
            if(inWord) for (int i = 0; i < arrLen; i++ ) if (newWord[i] == display || ( special && display == 93 ) || ( special && display == 148 )) tmpInWord = false;
            if(!inWord) for (int i = 0; i < arrLen; i++) if (newWord[i] == display || display == 39 || ( special && display == 93 ) || ( special && display == 152 ) || ( special && display == 153 ) || ( special && display == 148 )) tmpInWord = false;
            inWord = tmpInWord;
            if( !pastInWord && inWord ) w++;
            if( pastInWord && !inWord )
            {
                aFlag = true;
                eFlag = true;
                iFlag = true;
                oFlag = true;
                uFlag = true;
                cFlag = true;
            }
        }
        // UTF-8  -->  C3**
        if( display == 195) go = true;
        if( aFlag )
        {
            if( display == 65 || display == 97 || ( go && ( display == 128 || display == 129 || display == 130 || display == 131 || display == 160 || display == 161 || display == 162 || display == 163)))
            {
                a++;
                aFlag = false;
            }
        }
        if( eFlag )
        {
            if( display == 69 || display == 101 || ( go && ( display == 168 || display == 169 || display == 170 || display == 136 || display == 137 || display == 138 )))
            {
                e++;
                eFlag = false;
            }
        }
        if( iFlag )
        {
            if( display == 73 || display == 105 || ( go && ( display == 172 || display == 173 || display == 140 || display == 141 )))
            {
                i++;
                iFlag = false;
            }
        }
        if( oFlag )
        {
            if( display == 79 || display == 111 || ( go && ( display == 178 || display == 179 || display == 180 || display == 181 || display == 146 || display == 147 || display == 148 || display == 149)))
            {
                o++;
                oFlag = false;
            }
        }
        if( uFlag )
        {
            if( display == 85 || display == 117 || ( go && ( display == 185 || display == 186 || display == 153 || display == 154 )))
            {
                u++;
                uFlag = false;
            }
        }
        if( cFlag )
        {
            if(( go && ( display == 167 || display == 135 )))
            {
                c++;
                cFlag = false;
            }
        }
        if( display != 195 ) go = false;   

        // displaying every characters
        //printf("%x ", display);
        prev = display;
    }
    out->fileID = in->fileID;
    out->nWords = w;
    out->a = a;
    out->e = e;
    out->i = i;
    out->o = o;
    out->u = u;
    // printf("\nA: %i \n", a);
    // printf("\nE: %i \n", e);
    // printf("\nI: %i \n", i);
    // printf("\nO: %i \n", o);
    // printf("\nU: %i \n", u);
    // printf("Words: %i \n", w);
}
