import sys
import subprocess

"""

1:1	W5	160 	146512	314.54
3:2	W4	168 	151932	295.71
0:1	W6	168 	143940	158.94
2:1	W2	176 	158140	307.17
2:1	W7	184 	164720	320.95
2:1	W10	184 	164539	327.7
1:1	W8	186 	150618	344.07
3:1	W11	190 	171091	356.01
3:1	W3	192 	172252	344.37
3:1	W9	192 	171715	297.7
4:1	W12	194 	173698	301.38


"""


def DRC_setting_map(ipt):
    # ratio(read, write)
    ratio_fmt = lambda a, b: 1.0 * (b / a)

    if ipt < ratio_fmt(4, 1):
        return 168

    if ipt < ratio_fmt(3, 1):
        return 190

    if ipt < ratio_fmt(2, 1):
        return 187

    if ipt < ratio_fmt(3, 2):
        return 168

    if ipt < ratio_fmt(1, 1):
        return 160

    return 168


class MovingAverageQ(list):
    length = 2

    def set_length(self, v):
        self.length = v

    def append(self, t):
        super().append(t)
        if len(self) > self.length:
            del (self[0])

    def moving_average(self):
        return 1.0 * sum(self) / self.length


class DRC:

    def __init__(self, cmd="./IMC-raw.x", q=2, window=2):
        self.cmd = cmd
        self.q = []
        for i in range(q):
            q = MovingAverageQ()
            q.set_length(window)
            self.q.append(q)

    def __iter__(self):
        self.process = subprocess.Popen(self.cmd, stderr=subprocess.PIPE,
                                        shell=True, stdout=subprocess.PIPE)

        print("CMD: %s is running!" % self.cmd)

        for i in self.process.stdout:
            values = self.filter(i)
            if values is not None:
                yield self.controller(values)

            if self.process.poll():
                break

    def filter(self, raw):
        line = raw.decode()
        if line.startswith("W/R"):
            # val = [float(f) for f in line[5:-2].split(",")]
            # return val
            line = line[4:-1].split(",")
            ratio = [line[0], line[3]]

            val = []
            #for i, f in enumerate(line[5:-2].split(",")):
            for i, f in enumerate(ratio):
                """
                moving average queue
                """
                self.q[i].append(float(f))
                print("Queue for SKT#%d is %s\n" %(i, self.q[i]))
                val.append(self.q[i].moving_average())

            return val

    def controller(self, ratio):
        for i, v in enumerate(ratio):
            drc_value = DRC_setting_map(v)
            print("Set MSR on SKT#%d, ratio=%s, set value %s" % (
                i, v, drc_value))
            # todo: add real method here
            # set_DRC(DRC_setting_map(v))
            print("Fake Done!")

        return ratio


if __name__ == "__main__":
    drc = DRC(
        #'./IMC-raw.x -e imc/config=0x000000000000cf05,name="UNC_M_CAS_COUNT.RD" -e imc/config=0x000000000000f005,name="UNC_M_CAS_COUNT.WR" -i 4 -d 2')
        './IMC-raw.x -e imc/config=0x000000000000f005,name=UNC_M_CAS_COUNT.WR -e imc/config=0x000000000000cf05,name=UNC_M_CAS_COUNT.RD  -e imc/config=0x0000000000000082,name=UNC_M_WPQ_OCCUPANCY_PCH0 -e imc/config=0x0000000000000080,name=UNC_M_RPQ_OCCUPANCY_PCH0  -d 2')
    for i in drc:
        print()
