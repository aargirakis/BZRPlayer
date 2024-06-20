#ifndef IGBLOCK_H
#define IGBLOCK_H

class IGBlock
{
    friend class IGPlayer;
public:
    IGBlock();
private:
      int flags;
      int pointer;
      int position;
      int amount;
      int negative;
      int positive;
      int delay1;
      int delay2;
	  void reset();
};

#endif // IGBLOCK_H
