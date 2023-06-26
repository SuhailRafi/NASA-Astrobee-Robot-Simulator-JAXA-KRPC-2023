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

function plotError(telemData1, telemData2, units, titleStr, legend1, legend2,style2, t0)
% Prep for plot function in the telem class

error_data = telemData1 - telemData2;

if t0 > 0
    t1 = telemData1.time - t0;
    t2 = telemData2.time - t0;
    te = error_data.time - t0;
elseif t0 == 0
    t1 = telemData1.time;
    t2 = telemData2.time;
    te = error_data.time;
else
    t1 = datetime(telemData1.time, 'ConvertFrom', 'PosixTime');
    t2 = datetime(telemData2.time, 'ConvertFrom', 'PosixTime');
    te = datetime(error_data.time, 'ConvertFrom', 'PosixTime');
end
    
figure;%(5); clf;
subplot(2,1,1);
plot(t1, telemData1.data)
hold_on;
plot(t2, telemData2.data, style2)
title(titleStr); grid on;
ylabel(units); xlabel('seconds')
if size(telemData1.data, 2)==1 && size(telemData2.data, 2)==1
    legend(legend1, legend2);
else
    legend([legend1 '_x'], [legend1 '_y'], [legend1 '_z'], [legend2 '_x'], [legend2 '_y'], [legend2 '_z']);
end


subplot(2,1,2);
plot(te, error_data.data);
hold on; magLine = plot(te, rssrow(error_data.data), '--k');
legend(magLine, '|error|');
title([titleStr ' Error']); grid on;
ylabel(units); xlabel('seconds')

% Set the figure title
r = gca;
set(gcf, 'Name', r.Title.String)