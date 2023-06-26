% Copyright (c) 2017, United States Government, as represented by the
% Administrator of the National Aeronautics and Space Administration.
%
% All rights reserved.
%
% The Astrobee platform is licensed under the Apache License, Version 2.0
% (the "License"); you may not use this file except in compliance with the
% License. You may obtain a copy of the License at
%
%     http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
% WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
% License for the specific language governing permissions and limitations
% under the License.

function plotQuatsError(data1, data2, titleStr, legend1, legend2,style2, t0)
% Prep for plot function in the telem class
figure;%(5); clf;
% Combine calcs into analysis file 
R1 = data1.data.convert_to_eulers;
R2 = data2.data.convert_to_eulers;
qError = data1-data2;

if t0 > 0
    t1 = data1.time - t0;
    t2 = data2.time - t0;
    te = qError.time - t0;
elseif t0 == 0
    t1 = data1.time;
    t2 = data2.time;
    te = qError.time;
else
    t1 = datetime(data1.time, 'ConvertFrom', 'PosixTime');
    t2 = datetime(data2.time, 'ConvertFrom', 'PosixTime');
    te = datetime(qError.time, 'ConvertFrom', 'PosixTime');
end


subplot(2,1,1);
plot(t1, R1*180/pi)
hold_on;
plot(t2, R2*180/pi, style2)
title(titleStr); grid on;
ylabel('Euler Angles, Deg'); xlabel('seconds')
legend([legend1 '_\theta'], [legend1 '_\phi'], [legend1 '_\psi'], [legend2 '_\theta'], [legend2 '_\phi'], [legend2 '_\psi']);


subplot(2,1,2);
plot(te, qError.data*180/pi);
title([titleStr ' Error']); grid on;
ylabel('Deg'); xlabel('seconds')

% Set the figure title
r = gca;
set(gcf, 'Name', r.Title.String)
