This is a small class designed for 2x2 display from SB components.
is is only to be used on a 240x240 display.
It will only work with internet connection
at line 64 change status = WiFi.begin("ssid", "password"); to your network settings.
The class is designed to use heap not stack and is running on core1.
Remember usb and network irq are always going to core0.
the constructor is on line 276 and the initialisation of the 4 clocks is on line 358
at line 387 to 393 is time update function.

After ending rtc settings from the net core 0 is free to use. 
it is only porsible to use half hours via 
clk4->changeUtcOffset( utcNewDelhi , utcNewDelhiMin );
clk4->settext(utcNewDelhitxt);
But I cant get it to work in the right way , It loose variabel adresse after some time (lcd driver "I thing")
