#define Ndec 256
float RX_DEC_FIR_coeffs[256]={
0.000000e+00,
1.332701e-09,
6.718274e-09,
1.889618e-08,
3.832024e-08,
5.901446e-08,
6.618739e-08,
3.768148e-08,
-4.965679e-08,
-2.092391e-07,
-4.321026e-07,
-6.765206e-07,
-8.648021e-07,
-8.916609e-07,
-6.463421e-07,
-4.701460e-08,
9.185607e-07,
2.156328e-06,
3.446052e-06,
4.450801e-06,
4.764217e-06,
3.996078e-06,
1.885935e-06,
-1.576700e-06,
-6.056126e-06,
-1.083211e-05,
-1.484870e-05,
-1.686572e-05,
-1.570610e-05,
-1.056607e-05,
-1.328901e-06,
1.119443e-05,
2.517806e-05,
3.791188e-05,
4.617897e-05,
4.685613e-05,
3.765503e-05,
1.786635e-05,
-1.106509e-05,
-4.530915e-05,
-7.889997e-05,
-1.045249e-04,
-1.148012e-04,
-1.038675e-04,
-6.900554e-05,
-1.194163e-05,
6.053007e-05,
1.368826e-04,
2.022791e-04,
2.410006e-04,
2.395599e-04,
1.899728e-04,
9.251909e-05,
-4.268956e-05,
-1.958925e-04,
-3.399028e-04,
-4.442796e-04,
-4.809318e-04,
-4.302467e-04,
-2.865670e-04,
-6.177619e-05,
2.140643e-04,
4.956504e-04,
7.289334e-04,
8.604403e-04,
8.479825e-04,
6.708109e-04,
3.370914e-04,
-1.132096e-04,
-6.112990e-04,
-1.068672e-03,
-1.391462e-03,
-1.497942e-03,
-1.336171e-03,
-8.983398e-04,
-2.285267e-04,
5.785571e-04,
1.388815e-03,
2.049158e-03,
2.414302e-03,
2.375609e-03,
1.886672e-03,
9.802899e-04,
-2.275263e-04,
-1.548910e-03,
-2.750603e-03,
-3.592339e-03,
-3.871291e-03,
-3.464897e-03,
-2.363744e-03,
-6.871465e-04,
1.323572e-03,
3.335428e-03,
4.974830e-03,
5.892192e-03,
5.829664e-03,
4.679756e-03,
2.523036e-03,
-3.644341e-04,
-3.538642e-03,
-6.449067e-03,
-8.526600e-03,
-9.284999e-03,
-8.419123e-03,
-5.882003e-03,
-1.924736e-03,
2.911935e-03,
7.858149e-03,
1.202274e-02,
1.453486e-02,
1.469902e-02,
1.213878e-02,
6.904218e-03,
-4.780789e-04,
-9.026127e-03,
-1.739790e-02,
-2.405250e-02,
-2.745767e-02,
-2.631538e-02,
-1.977278e-02,
-7.585759e-03,
9.791963e-03,
3.121173e-02,
5.492923e-02,
7.879043e-02,
1.004814e-01,
1.178091e-01,
1.289745e-01,
1.328008e-01,
1.288838e-01,
1.176435e-01,
1.002696e-01,
7.856896e-02,
5.473624e-02,
3.108015e-02,
9.743801e-03,
-7.543114e-03,
-1.964771e-02,
-2.613040e-02,
-2.724530e-02,
-2.384949e-02,
-1.723876e-02,
-8.937176e-03,
-4.730285e-04,
6.826376e-03,
1.199328e-02,
1.451234e-02,
1.433987e-02,
1.185282e-02,
7.741439e-03,
2.866586e-03,
-1.893367e-03,
-5.781870e-03,
-8.269662e-03,
-9.113373e-03,
-8.362728e-03,
-6.320367e-03,
-3.465402e-03,
-3.566200e-04,
2.467051e-03,
4.572398e-03,
5.691522e-03,
5.748093e-03,
4.849366e-03,
3.248747e-03,
1.288152e-03,
-6.682234e-04,
-2.296800e-03,
-3.364036e-03,
-3.755528e-03,
-3.482046e-03,
-2.663939e-03,
-1.498851e-03,
-2.199869e-04,
9.469982e-04,
1.821030e-03,
2.290963e-03,
2.326234e-03,
1.972660e-03,
1.335771e-03,
5.559561e-04,
-2.193985e-04,
-8.616583e-04,
-1.280412e-03,
-1.434072e-03,
-1.330854e-03,
-1.021131e-03,
-5.835299e-04,
-1.079591e-04,
3.211329e-04,
6.383985e-04,
8.061705e-04,
8.171509e-04,
6.915186e-04,
4.696981e-04,
2.026316e-04,
-5.841116e-05,
-2.706477e-04,
-4.058734e-04,
-4.531503e-04,
-4.181110e-04,
-3.194895e-04,
-1.838975e-04,
-4.002444e-05,
8.663016e-05,
1.776445e-04,
2.237094e-04,
2.247425e-04,
1.883655e-04,
1.272822e-04,
5.620105e-05,
-1.107070e-05,
-6.387288e-05,
-9.598772e-05,
-1.059175e-04,
-9.627335e-05,
-7.254505e-05,
-4.158523e-05,
-1.013694e-05,
1.633660e-05,
3.436338e-05,
4.267345e-05,
4.196849e-05,
3.438027e-05,
2.278113e-05,
1.010499e-05,
-1.196643e-06,
-9.490252e-06,
-1.406936e-05,
-1.506595e-05,
-1.322524e-05,
-9.617922e-06,
-5.359670e-06,
-1.390521e-06,
1.657061e-06,
3.497121e-06,
4.151416e-06,
3.860205e-06,
2.973528e-06,
1.850194e-06,
7.832353e-07,
-3.980811e-08,
-5.429466e-07,
-7.422680e-07,
-7.124248e-07,
-5.505701e-07,
-3.466484e-07,
-1.650235e-07,
-3.837418e-08,
2.842000e-08,
4.851147e-08,
4.189804e-08,
2.641565e-08,
1.291744e-08,
4.874207e-09,
1.165619e-09
};