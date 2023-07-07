At one point in the project we were unsure if the SCI block was being transmitted correctly. This lead to us attempting to decode the SCI of a known good message (as transmitted by our OBU) and our message, and then comparing the two. According to [Hajisami](https://ieeexplore.ieee.org/document/9866543) (Section III.A.1), the SCI is always encoded in QPSK.

Here were the steps we took:
1. Repeatedly transmit a BSM message on the air from our OBU.
2. Set the Spectrum analyzer parameters:
	- Center Frequency: `5.915e9`
	- Bandwidth: `5 MHz` (to try and capture only 1 message immediately adjacent to the center frequency)
	- Triggered mode
3. Capture a transmission and save it to a [.mat](./2023-06-29%20OBU%20message%20at%205.915.mat) file.
	- File, Save As.
        - Data types: IQ records.
        - Data length: Analysis Only.
        - Data records: Current acquisition. 
4. Use the python script, [mat_converter.py](./mat_converter.py) to convert that `.mat` into a [.cf64](./2023-06-29_OBU.cf64).
    - You can optionally verify the result looks correct by installing [inspectrum](https://github.com/miek/inspectrum) and opening the file with that.
5. Load the `.cf64` into GnuRadio (See [PC-5_mode-4_SCI_decoding.grc](./PC-5_mode-4_SCI_decoding.grc))
6. Trim the frequency range using filters to only capture the SCI.

... and that's as far as we got. The result didn't look right in the constellation. The likely reason why is because the QPSK constellation is encoded using SC-FDMA, and we haven't dealt with that. 

One thing we discovered about the particular capture we did was that, while the Spectrum analyzer software reported we had captured at a sampling rate of 12.5 MHz, the metadata of the capture indicated it was actually 7 MHz. We found that the capture looked most accurate at a 7 MHz sampling rate (i.e. transmission was ~2 MHz wide, ~1 ms long)

I found a [2014 thesis paper](https://run.unl.pt/bitstream/10362/12251/1/Cunha_2014.pdf) that gives instructions about how to create a custom GNU Radio module for decoding SC-FDMA, but it was complex and difficult for me to understand.

I also ran the `./pssch_ue` script in `-v` verbose mode (receiving from a B200mini transmitting my message), and the verbose option didn't tell me much more. I wrote out the verbose log into [b200-to-b200.log](./b200-to-b200.log)