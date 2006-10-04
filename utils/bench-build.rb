#!/usr/bin/ruby

require 'benchmark'

DISK='sdb1'

def init( cmd )
    Dir.chdir "/mnt"
    `umount /mnt/md &> /dev/null`
    `#{cmd} &> /dev/null`
    `mount /dev/#{DISK} /mnt/md`
    Dir.mkdir "/mnt/md/build"
    Dir.chdir "/mnt/md"
    `svn co "http://snoopy/code/svn/spinny"`
end

n = 10

def make
    `rm -rf /mnt/md/build`
    Dir.mkdir "/mnt/md/build"
    Dir.chdir "/mnt/md/build"
    `cmake ../spinny`
    `make -j4 2> /dev/null`
    Dir.chdir "/mnt/md"
    `rm -rf /mnt/md/build`
end

Benchmark.bm(15) do |x|
    init "mkfs.reiserfs -f -q /dev/#{DISK}"
    x.report("Reiserfs") { n.times{ make } }
    init "mkfs.xfs -f -q /dev/#{DISK}"
    x.report("XFS"){ n.times{ make } }
    init "mkfs.ext3  /dev/#{DISK}"
    x.report("ext3"){ n.times{ make } }
    init "mkfs.ext2  /dev/#{DISK}"
    x.report("ext2"){ n.times{ make } }
    init "mkfs.jfs -q /dev/#{DISK}"
    x.report("jfs"){ n.times{ make } }
end


__END__

raid0:
                     user     system      total        real
Reiserfs         0.020000   0.020000 1110.870000 (657.285619)
XFS              0.010000   0.030000 1097.710000 (639.880578)
ext3             0.020000   0.040000 1094.180000 (637.993840)
ext2             0.030000   0.030000 1092.290000 (665.240184)
jfs              0.030000   0.020000 1089.180000 (596.621438)


raid5:
                     user     system      total        real
Reiserfs         0.020000   0.030000 1182.970000 (715.958916)
XFS              0.030000   0.040000 1163.440000 (731.595719)
ext3             0.030000   0.040000 1163.080000 (764.140518)
ext2             0.020000   0.030000 1157.240000 (687.272438)
jfs              0.020000   0.040000 1164.570000 (825.286137)

Single
Reiserfs         0.020000   0.020000 1119.630000 (616.229999)
XFS              0.020000   0.020000 1109.100000 (658.996869)
ext3             0.030000   0.030000 1108.170000 (603.970052)
ext2             0.010000   0.030000 1100.430000 (590.514393)
jfs              0.020000   0.030000 1121.860000 (643.862332)

Reiserfs         0.020000   0.020000 1124.360000 (610.244014)
XFS              0.030000   0.020000 1119.620000 (652.421709)
ext3             0.020000   0.030000 1108.700000 (602.408614)
ext2             0.020000   0.040000 1107.280000 (606.553268)
jfs              0.030000   0.020000 1109.660000 (622.876628)
