# ftDuino case designs

These are mesh files for the ftDuino 1.1 case. At least `bottom.stl` and
one of `top_with_isp.stl` or `top_without_isp.stl` are needed.

| ![bottom case](images/bottom.png) | ![top case with ISP connector opening](images/top_with_isp.png) | ![top case without ISP connector opening](images/top_without_isp.png) |
|:---:|:---:|:---:|
| [View `bottom.stl`](bottom.stl) | [View `top_with_isp.stl`](top_with_isp.stl) | [View `top_without_isp.stl`](top_without_isp.stl) |
| [Download `bottom.stl`](bottom.stl?raw=true) | [Download `top_with_isp.stl`](top_with_isp.stl?raw=true) | [Download `top_without_isp.stl`](top_without_isp.stl?raw=true) |

The `top_with_isp.stl` is to be used if the ISP connector is to
be mounted. The `top_without_isp.stl` is for boards without ISP
connector. The ISP connector hole in `top_with_isp.stl` can alternally
be closed with the `isp_cap.stl`.

| ![ISP connector cap](images/isp_cap.png) |
|:---:|
| [View `isp_cap.stl`](isp_cap.stl) |
| [Download `isp_cap.stl`](isp_cap.stl?raw=true) |

The `i2c_protector.stl` is a small piece that fits into the 6 pin
I²C connector if it's not in use and prevents it from accidentially
being shortened or otherwise damaged.

| ![I²C protector](images/i2c_protector.png) |
|:---:|
| [View `i2c_protector.stl`](i2c_protector.stl) |
| [Download `i2c_protector.stl`](i2c_protector.stl?raw=true) |

The `reset_button.stl` reset button is required if a 4.3mm tactile
switch is used as a reset button. It's not needed if a 7 or 9mm
version is used.

| ![Reset button](images/reset_button.png) |
|:---:|
| [View `reset_button.stl`](reset_button.stl) |
| [Download `reset_button.stl`](reset_button.stl?raw=true) |
