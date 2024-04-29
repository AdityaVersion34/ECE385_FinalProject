# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\Aditya\ece385\final_project\ddr3_test\ddr3test\ddr3test_workspace\mb_ddr3_top\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\Aditya\ece385\final_project\ddr3_test\ddr3test\ddr3test_workspace\mb_ddr3_top\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {mb_ddr3_top}\
-hw {C:\Users\Aditya\ece385\final_project\ddr3_test\ddr3test\mb_ddr3_top.xsa}\
-out {C:/Users/Aditya/ece385/final_project/ddr3_test/ddr3test/ddr3test_workspace}

platform write
domain create -name {standalone_microblaze_0} -display-name {standalone_microblaze_0} -os {standalone} -proc {microblaze_0} -runtime {cpp} -arch {32-bit} -support-app {dhrystone}
platform generate -domains 
platform active {mb_ddr3_top}
platform generate -quick
platform clean
platform generate
platform config -updatehw {C:/Users/Aditya/ece385/final_project/ddr3_test/ddr3test/mb_ddr3_top.xsa}
platform clean
platform generate
platform active {mb_ddr3_top}
platform config -updatehw {C:/Users/Aditya/ece385/final_project/raycast_ddr3_v1/ddr3test/mb_ddr3_top.xsa}
platform clean
platform generate
platform clean
platform generate
