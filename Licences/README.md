
	PPP-ARISEN: An open-source Precise Point Positioning software 
	            with Ambiguity Resolution for Interdisciplinary research of 
				Seismology/gEodesy/geodyNamics.

 Author: Chengfeng Zhang, Aizhi Guo
 Email : zhangcf@apm.ac.cn, guoaizhi@whigg.ac.cn/guoaizhi@apm.ac.cn

## English:

As the exponential growth of Global Navigation Satellite System (GNSS) 
station worldwide, GNSS record of time-series becomes a very important 
information source for many areas of earth science. Among the GNSS applications,
Precise Point Positioning (PPP) approach plays an essential role because 
of the ability of high precise absolute positioning with standalone receiver 
when the ambiguity of carrier phase is recovered with its integer property. 
Although many general-purpose and self-sufficient toolboxes are available for 
users of GNSS algorithms, a user-friendly toolbox focusing on signal extraction 
and applications and supporting products of different IGS Analysis Centers (ACs) 
is helpful for obtaining accurate results of robustness and self-assessment. 
For the geoscience community, we present a cross-platform open source PPP toolbox
PPP-ARISEN (Precise Point Positioning software with Ambiguity Resolution for 
Interdisciplinary research of Seismology/gEodesy/geodyNamics), which is 
compatible with both CODE and CNES products and realized ambiguity resolution
based on Integer Phase Clock (IPC) method with Satellite-to-satellite Single 
Difference (SSD) strategy. The toolbox can achieve millimeter-level precision 
for static positioning compared with IGS weekly SINEX result and centimeter-level
precision for kinematic mode, while the uniquely designed “Seismological” mode 
successfully captures clear dynamic signals induced by the earthquake with 
different products of IGS ACs. Moreover, we define and validate an effective 
index to judge the convergence status of PPP, which is called as Convergence 
Status Indicator based on ZWD Variances (CSI-ZWDV). The index is superior to 
previous experimental parameters of determining when to search for the integer
ambiguities, along with its more explicit physical meaning. 