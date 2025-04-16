# 🕒 ClockHack2313  
## — Educational hack of a DIY clock kit based on ATtiny2313

**ClockHack2313** is an educational project that enables **cheap** and effective learning of AVR microcontrollers (specifically ATtiny2313), 7-segment display control, button input reading, sound generation, and timer usage.

I used this project to teach school students soldering and programming. Therefore, it had to be simple, inexpensive, and accessible for almost everyone.

<details>
<summary>The main challenge I faced</summary>

> While teaching the course, I encountered the problem of students having very different skill levels. The typical linear approach to learning (step-by-step progression with dependencies) simply didn’t work.
> 
> So I created conditions that work for a diverse group. More advanced students had time to solder the board, play with the simulation, and modify the code — adding their own IO and interrupt logic. Others could focus on either just assembling the clock, or at least experimenting with the working simulation to see how it functions.

</details>

---

<details open>
<summary><strong>The base is a cheap DIY clock kit</strong></summary>
<a href="img-vid/ClockHack2313_main.jpg" target="_blank"><img src="img-vid/ClockHack2313_main.jpg" width="17%"></a>

It’s a common soldering kit available on Amazon, AliExpress, etc. — inexpensive and widely available.

The board is made of good quality PCB material (at least the ones I received), and even beginner students couldn't damage the traces. That’s crucial for learners who are just discovering how to solder.

It also uses a DIP socket for the microcontroller, which is much easier to solder than SMD chips.

The kit comes with a pre-programmed 89C2051 microcontroller that runs the clock firmware. It is inserted into a DIP-20 socket, so no soldering is needed. Some students stopped at this point and used it as a basic clock.

For further learning, I replaced this microcontroller with a pin-compatible **ATtiny2313**, which has a more modern architecture. (I believe this is a better starting point for serious AVR learning.)

Moreover, many **Arduino boards** are also based on the AVR architecture, making this knowledge transferable to more complex projects.

💡 **Important note:** the only hardware modification required is related to the reset pin. The RST pin on 89C2051 is active high, while ATtiny2313 uses an **inverted** reset signal. So a resistor must be moved from GND to VCC.

</details>

---

<details open>
<summary><strong>A demo firmware was written in C</strong></summary>
<a href="img-vid/AVR_studio.png" target="_blank"><img src="img-vid/AVR_studio.png" width="22%"></a>

Located in the `firmware` folder, written using **AVRStudio** (older version) and compiled with **AVR-GCC**.

The code includes basic features: character output to the display, button interaction, contact debounce, buzzer beeping, and interrupt usage. These are fundamental exercises for working with microcontrollers.

</details>

---

### 🎯 Project Goals

- Provide a **cheap and simple hardware platform** for learning AVR soldering and programming
- Teach how to solder electronic kits
- Enable **full simulation in Proteus** — no soldering needed
- Build **low-level programming skills and microcontroller understanding**
- Support both beginners and more advanced students with meaningful tasks

---

### 🧰 Project Includes

- A modified clock kit with the microcontroller replaced by **ATtiny2313**
- A demo firmware in **C** for AVR-GCC (display, buttons, buzzer, timers)
- Full **simulation in Proteus**, including step-by-step debugging
- **Photos of the assembled board**, Proteus and AVR Studio screenshots
- Ready-to-use `.hex` firmware file
- AVR Studio project (legacy version)

---

### 🧠 What You Will Learn

- Working with GPIO: display segments, buttons, and buzzer
- Delays and timer interrupts
- Simple text effects like **scrolling text**
- Basic logic for games and animations
- **Proteus simulation and debugging**
- Structure of a C program for AVR
- Using `ISR` (interrupt service routines), button debounce logic, GPIO switching

---

### ⚙️ Hardware and Components

- 4-digit **7-segment common cathode** display  
  (Anodes on PB0–PB7, cathodes on PD0, PD1, PD2, PD6)
- 2 push buttons (PD4 and PD5)
- Buzzer with transistor (PD3)
- Microcontroller: **ATtiny2313** in DIP-20 (replacing 89C2051 from the original kit)
- Pull-up resistor on RST moved to **VCC** (due to reset polarity)
- PCB from a generic DIY clock kit

---

### 🛠️ How to Start

1. Get a DIY clock kit
2. Replace the 89C2051 chip with an **ATtiny2313**
3. **Move the pull-up resistor on the RST pin** from GND to VCC
4. Flash the `.hex` firmware using USBasp or any compatible programmer
5. Power up — and watch the demo in action!

_Or_ — simply open the `.dsn` file in **Proteus 8**, and run it without soldering anything!

---

### 📸 Screenshots

<a href="img-vid/ClockHack2313_main.jpg" target="_blank"><img src="img-vid/ClockHack2313_main.jpg" width="17%"></a> <a href="img-vid/Proteus_simulator.png" target="_blank"><img src="img-vid/Proteus_simulator.png" width="25%"></a> <a href="img-vid/AVR_studio.png" target="_blank"><img src="img-vid/AVR_studio.png" width="22%"></a>

---

### 📜 License

Free to use for educational purposes with proper attribution.

[Creative Commons Attribution 4.0 International (CC BY 4.0)](LICENSE)

The author is not responsible for what you learn — or solder 😉
