# TOPICS: U-Boot qcom/integration

Base: main f97bb5c1ad78. This branch merges every qcom/<platform> branch
below onto the base (a branch that another one builds on comes in
through that branch's merge); each merge commit message records its
conflict resolutions and adaptations.

| platform | branch | depends on | series | upstream status |
|---|---|---|---|---|
| glymur | qcom/glymur | main | Glymur CRD support, memory carve-outs, boot environment | local |
| mahua | qcom/mahua | glymur | Mahua CRD | local |
| pakala | qcom/pakala | main | SM8750 MTP | local |
| kaanapali | qcom/kaanapali | pakala | SM8850 MTP | local |
| hamoa | qcom/hamoa | main | X1E80100 IoT EVK, RCG divider fix | RCG fix posted on the fork; rest local |
| purwa | qcom/purwa | hamoa | X1P42100 IoT EVK + memory carve-outs | local |
| monaco | qcom/monaco | main | VENTUNO Q + RCG fix | v3 drafted, not sent |
| agatti | qcom/agatti | main | UNO Q board, OP-TEE memory reservation | reservation new |
| talos | qcom/talos | main | QCS615 Ride, l17a always on (UFS and eMMC share it) | l17a new |
| lanai | qcom/lanai | main | SM8650 QRD | local |
| kodiak | qcom/kodiak | main | RB3 Gen 2 fixes (RTC, UFS cache, select-utmi) | local |
| lemans | qcom/lemans | main | IQ-9075 EVK fixes | local (no Lemans SPL on main) |

## LAVA results

U-Boot 82fbe4d77385 with the matching TF-A and OP-TEE integration
builds; one fresh firmware fingerprint per build, read back on the cold
boot and, where the board comes back, the second boot; the OSS suite
minus ACS, compared case by case with the platform baseline. Every board
ends on stock firmware; Monaco through a separate restore job, because
its warm reboot hangs. Jobs:
https://lava.infra.foundries.io/scheduler/job/<id>.

| platform | job(s) | result vs baseline |
|---|---|---|
| pakala | 511913 | equal to 507296 |
| hamoa | 511915 | equal to 505458 |
| purwa | 511910 | equal to 507756 |
| glymur | 511917 | equal to 510585 |
| mahua | 511918 | equal to 510593 |
| kaanapali | 511465 (U-Boot 1565f43e197d) | equal to 505311 |
| kodiak | 511914, 511916 | equal to 502713 except a big-core cpufreq runtime-ratio board flake |
| agatti | 511909, 511900 | equal to 490882 and 490883 |
| monaco | 511890, 511893 | equal to 502190 (gpu-tflite x2 now pass) up to the warm reboot, which hangs in firmware (SAIL acknowledge, open) |
| talos | 511898, 511901 | as 492393 except no OSM cpufreq and a kvm-unit-tests PMU timing race that fails as often with the previous firmware (per run 2.2 % vs 3.1 %) |
| lemans | 511911, 511912 | U-Boot proper: no regression vs 509671 and 509672; SPL not gated |
| lanai | | untested: no LAVA board |
