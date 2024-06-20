#include "FileLoader.h"
#include "D1Player.h"
#include "D2Player.h"
#include "IGPlayer.h"
#include "JBPlayer.h"
#include "RJPlayer.h"
#include "BDPlayer.h"
#include "FXPlayer.h"
#include "BPPlayer.h"
#include "S2Player.h"
#include "S1Player.h"
#include "MGPlayer.h"
#include "RHPlayer.h"
#include "FEPlayer.h"
#include "JHPlayer.h"
#include "DWPlayer.h"
//#include "STPlayer.h"
//#include "MKPlayer.h"
//#include "HMPlayer.h"
#include "PTPlayer.h"
#include <iostream>
FileLoader::FileLoader()
{
    force=0;
}
void FileLoader::setForcePlayer(int value)
{
    force=value;
}

AmigaPlayer* FileLoader::load(void* data, unsigned long int length, const char* filename)
{

    if(length<3) return 0;
    unsigned char *d = static_cast<unsigned char*>(data);


    //    if(force==0 || force==2)
    //    {
    //        if (length > 2105) {

    //            if(
    //                    (d[1080]=='M' && d[1081]=='.' && d[1082]=='K' && d[1083]=='.') ||
    //                    (d[1080]=='F' && d[1081]=='L' && d[1082]=='T' && d[1083]=='4')
    //                    )
    //            {
    //                MKPlayer* player = new MKPlayer(new Amiga());
    //                player->load((signed short*)data,length);
    //                if (player->version)
    //                {
    //                    return player;
    //                }
    //            else
    //            {
    //                delete player;
    //            }
    //            }
    //            else if ((d[1080]=='F' && d[1081]=='E' && d[1082]=='S' && d[1083]=='T')  ||
    //                     (d[1080]=='M' && d[1081]=='&' && d[1082]=='K' && d[1083]=='!') )
    //            {
    //                HMPlayer* player = new HMPlayer(new Amiga());
    //                player->load((signed short*)data,length);
    //                if (player->version)
    //                {
    //                    return player;
    //                }
    //            else
    //            {
    //                delete player;
    //            }
    //            }
    //        }
    //    }

    //    if(force==0 || force==1)
    //    {
//    if (length > 2105)
//    {


//        PTPlayer* player = new PTPlayer(new Amiga());
//        player->load((signed short*)data,length);
//        if (player->getVersion())
//        {
//            return player;
//        }
//        else
//        {
//            delete player;
//        }
//    }



    if(length > 1685)
    {
        if(
                (d[60]=='S' && d[61]=='O' && d[62]=='N' && d[63]=='G') ||
                (d[124]=='S' && d[125]=='O' && d[126]=='N' && d[127]=='G') ||
                (d[124]=='S' && d[125]=='O' && d[126]=='3' && d[127]=='1') )
        {
            FXPlayer* player = new FXPlayer(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }
        }

    }

    if(length > 4)
    {
        if(d[0]=='A' && d[1]=='L' && d[2]=='L' && d[3]==' ')
        {
            D1Player* player = new D1Player(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }

        }
    }

    {
        //Infogrames
        IGPlayer* player = new IGPlayer(new Amiga());
        player->load((signed short*)data,length,filename);
        if (player->getVersion())
        {
            return player;
        }
        else
        {
            delete player;
        }
    }

    if(d[0]=='R' && d[1]=='J' && d[2]=='P' && d[4]=='S' && d[5]=='M' && d[6]=='O' && d[7]=='D')
    {
        //Richard Joseph
        RJPlayer* player = new RJPlayer(new Amiga());
        player->load((signed short*)data,length,filename);
        if (player->getVersion())
        {
            return player;
        }
        else
        {
            delete player;
        }
    }
    {

        //Jason Brooke
        JBPlayer* player = new JBPlayer(new Amiga());
        player->load((signed short*)data,length);
        if (player->getVersion())
        {
            return player;
        }
        else
        {
            delete player;
        }
    }

    if(d[0]==0x60 && d[1]==0x00)
    {

        //Ben Daglish
        BDPlayer* player = new BDPlayer(new Amiga());
        player->load((signed short*)data,length);
        if (player->getVersion())
        {
            return player;
        }
        else
        {
            delete player;
        }
    }
    if(length > 3018)
    {
        if(d[3014]=='.' && d[3015]=='F' && d[3016]=='N' && d[3017]=='L')
        {
            D2Player* player = new D2Player(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }

        }
    }

    if (length > 30)
    {

        if(
                (d[26]=='B' && d[27]=='P' && d[28]=='S' ) ||
                (d[26]=='V' && d[27]=='.' && d[28]=='2' ) ||
                (d[26]=='V' && d[27]=='.' && d[28]=='3' ) )
        {
            BPPlayer* player = new BPPlayer(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }
        }
    }

    if(length > 10)
    {
        if(d[0]==' ' && d[1]=='M' && d[2]=='U' && d[3]=='G' && d[4]=='I' && d[5]=='C' && d[6]=='I' && d[7]=='A' && d[8]=='N')
        {
            MGPlayer* player = new MGPlayer(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }

        }
    }

    if (length > 86) {

        if(d[58]=='S' && d[59]=='I' && d[60]=='D' && d[61]=='M' && d[62]=='O' && d[63]=='N' && d[64]==' ' &&
                d[65]=='I' && d[66]=='I' && d[67]==' ' && d[68]=='-' && d[69]==' ' && d[70]=='T' && d[71]=='H' && d[72]=='E' &&
                d[73]==' ' && d[74]=='M' && d[75]=='I' && d[76]=='D' && d[77]=='I' && d[78]==' ' && d[79]=='V' && d[80]=='E' &&
                d[81]=='R' && d[82]=='S' && d[83]=='I' && d[84]=='O' && d[85]=='N')
        {
            S2Player* player = new S2Player(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }
        }
    }
    if (length > 2830) {
        if(d[0]==0x4e && d[1]==0xfa)
        {
            FEPlayer* player = new FEPlayer(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }
        }
    }

    if (length > 5220)
    {
        //don't try to load an sc68 file, sometimes they can be picked up as a Sidmon1
        if(!(d[0]=='S' && d[1]=='C' && d[2]=='6' && d[3]=='8' && d[4]==' ' && d[5]=='M' && d[6]=='u' && d[7]=='s' && d[8]=='i' && d[9]=='c'))
        {
            S1Player* player = new S1Player(new Amiga());
            player->load((signed short*)data,length);
            if (player->getVersion())
            {
                return player;
            }
            else
            {
                delete player;
            }
        }
    }

    if ((d[0]=='C' && d[1]=='O' && d[2]=='S' && d[3]=='O') || (d[0]==0x60 && d[1]==0x0) || (d[0]==0x60 && d[1]==0x2) || (d[0]==0x60 && d[1]==0xe) || (d[0]==0x60 && d[1]==0x16))
    {

        JHPlayer* player = new JHPlayer(new Amiga());
        player->load((signed short*)data,length);
        if (player->getVersion())
        {
            return player;
        }
        else
        {
            delete player;
        }
    }

    DWPlayer* player = new DWPlayer(new Amiga());
    //don't try to load an sc68 file, sometimes they can be picked up as a Whittaker
    if(!(d[0]=='S' && d[1]=='C' && d[2]=='6' && d[3]=='8' && d[4]==' ' && d[5]=='M' && d[6]=='u' && d[7]=='s' && d[8]=='i' && d[9]=='c'))
    {
        player->load((signed short*)data,length);
        if (player->getVersion())
        {
            return player;
        }
        else
        {
            delete player;
        }
    }

    if(d[0]==0x60 && d[1]==0 && d[2]==0)
    {
        RHPlayer* player = new RHPlayer(new Amiga());
        player->load((signed short*)data,length);
        if (player->getVersion())
        {
            return player;
        }
        else
        {
            delete player;
        }
    }
    //    if(force==0 || force==3)
    //    {
    //        if (length > 1625) {
    //            STPlayer* player = new STPlayer(new Amiga());
    //            player->load((signed short*)data,length);

    //            if (player->version)
    //            {
    //                return player;
    //            }
    //            else
    //            {
    //                delete player;
    //            }
    //        }
    //    }



    return 0;
}
