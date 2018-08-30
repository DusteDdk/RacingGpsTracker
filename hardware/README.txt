This is how stuff is put together.
I may do something more some time, but don't count on it.

     ^ = +5v, on the psu
     v =  0v, on the psu
    
     +-----+
     | GPS |
     |  +5 |--^ 
     | gnd |--v
     |  rx |---- tx on arduino
     |  tx |---- rx on arduino
     +-----+
    
     +------+
     |  SD  |
     |   +5 |--^
     |  gnd |--v
     | miso |---- 14 on arduino
     | mosi |---- 16 on arduino
     |  sck |---- 15 on arduino
     |   cs |----  9 on arduino
     +------+
    
     +-------+
     |arduino|
     |   vcc |--^ 
     |   gnd |--v  
     |     4 |->--- LED3 cathode, "recording" indicator
     |     5 |---- green wire, for uart mode (tx)
     |     6 |---- LED1 cathode, "waiting for fix" indicator
     |     7 |---- LED2 cathode, "got fix" indicator
     |     8 |---- yello wire, for uart mode (rx)
     +-------+
    
     +-----+
     | PSU |
     |  b+ |---- To the battery + terminal
     |  b- |---- To the battery - terminal
     |  p- |---- To toggle switch pole
     |   + |--^
     |   - |--v
     +-----+
    
     +-----------+
     | Toggle sw |
     | Terminal1 |---- To battery+
     | Terminal2 |---- To battery-
     +-----------+
    
     +---------+
     | Tact sw |
     |  Side 1 |--^
     |  Side 2 |------+------------- 3 on arduino
     +---------+      | +-----+
                      +-| 10k |--v
                        +-----+
    
     Repeat x3:
     +-----+
     | LED |     +------+
     |Anode|-----| 150r |--^
     +-----+     +------+
