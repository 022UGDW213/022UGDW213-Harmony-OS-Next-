

import React, { useState, useEffect, useRef, useCallback } from 'react';
import Draggable from 'react-draggable';

// Icon Imports
import FinderIcon from './components/icons/FinderIcon';
import WeatherIcon from './components/icons/WeatherIcon';
import InstallerIcon from './components/icons/InstallerIcon';
import PhotosIcon from './components/icons/PhotosIcon';
import MusicIcon from './components/icons/MusicIcon';
import CalendarIcon from './components/icons/CalendarIcon';
import SettingsIcon from './components/icons/SettingsIcon';
import SystemProfilerIcon from './components/icons/SystemProfilerIcon';
import CodeIcon from './components/icons/CodeIcon';
import HybridSimIcon from './components/icons/HybridSimIcon';
import SandboxIcon from './components/icons/SandboxIcon';
import CppIcon from './components/icons/CppIcon';
import AmazonMusicIcon from './components/icons/AmazonMusicIcon';


// --- App Content Components ---

const FinderContent = () => (
    <div className="grid grid-cols-4 gap-4 p-4 text-white">
        {[
            { icon: '📷', name: 'Lake Photos' },
            { icon: '📊', name: 'Weather Data' },
            { icon: '🗺️', name: 'Trail Maps' },
            { icon: '❄️', name: 'Snow Reports' },
            { icon: '🏔️', name: 'Mountain Info' },
        ].map(item => (
            <div key={item.name} className="flex flex-col items-center gap-2 p-2 rounded-lg hover:bg-white/10 transition-colors cursor-pointer">
                <div className="text-4xl">{item.icon}</div>
                <div className="text-xs text-center">{item.name}</div>
            </div>
        ))}
    </div>
);

const WeatherContent = () => (
    <div className="text-center p-6 text-white">
        <div className="text-5xl font-light">42°F</div>
        <div className="opacity-80 mt-2">Lake Tahoe, CA</div>
        <div className="mt-1">Partly Cloudy</div>
        <div className="grid grid-cols-2 gap-4 mt-6 text-left">
            <div className="bg-white/10 p-3 rounded-lg"><div>Wind</div><div>5 mph</div></div>
            <div className="bg-white/10 p-3 rounded-lg"><div>Humidity</div><div>65%</div></div>
            <div className="bg-white/10 p-3 rounded-lg"><div>Snow</div><div>24"</div></div>
            <div className="bg-white/10 p-3 rounded-lg"><div>Water Temp</div><div>41°F</div></div>
        </div>
    </div>
);

const InstallerContent = () => {
    const [progress, setProgress] = useState(0);
    const [status, setStatus] = useState('Idle');

    const handleCreate = () => {
        setStatus('Creating...');
        setProgress(0);
        const interval = setInterval(() => {
            setProgress(prev => {
                if (prev >= 100) {
                    clearInterval(interval);
                    setStatus('Completed');
                    return 100;
                }
                return prev + 1;
            });
        }, 50);
    };

    return (
        <div className="p-6 text-white flex flex-col h-full">
            <h2 className="text-lg font-semibold mb-4">Create Bootable Installer</h2>
            <div className="flex-grow">
                <p className="text-sm opacity-80 mb-2">Source: macOS_Tahoe_1.0.iso</p>
                <p className="text-sm opacity-80 mb-4">Destination: /Volumes/USB_Installer</p>
                <div className="w-full bg-white/10 rounded-full h-2.5 mb-2">
                    <div className="bg-blue-500 h-2.5 rounded-full" style={{ width: `${progress}%`, transition: 'width 0.1s' }}></div>
                </div>
                <p className="text-sm text-center opacity-90">{status} ({progress}%)</p>
            </div>
            <button onClick={handleCreate} className="w-full bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded transition-colors">
                Create
            </button>
        </div>
    );
};

const wallpapers = [
    { id: 'default', name: 'Tahoe Blue', style: { background: 'radial-gradient(circle at 30% 70%, rgba(74, 144, 226, 0.3) 0%, transparent 50%), radial-gradient(circle at 70% 30%, rgba(255, 255, 255, 0.2) 0%, transparent 50%), linear-gradient(135deg, #1e3c72 0%, #2a5298 100%)' } },
    { id: 'sunset', name: 'Sierra Sunset', style: { background: 'linear-gradient(135deg, rgb(255, 126, 95) 0%, rgb(254, 180, 123) 100%)' } },
    { id: 'forest', name: 'Pine Forest', style: { background: 'linear-gradient(135deg, rgb(19, 78, 94) 0%, rgb(113, 178, 128) 100%)' } },
    { id: 'night', name: 'Starry Night', style: { background: 'linear-gradient(135deg, rgb(35, 37, 38) 0%, rgb(65, 67, 69) 100%)' } },
    { id: 'lake', name: 'Emerald Bay', style: { backgroundImage: 'url(https://images.unsplash.com/photo-1549442191-744535639487?q=80&w=1280&auto=format&fit=crop)', backgroundSize: 'cover', backgroundPosition: 'center' } },
    { id: 'snow', name: 'Fresh Powder', style: { backgroundImage: 'url(https://images.unsplash.com/photo-1517282133856-1b5e2a2f8b5f?q=80&w=1280&auto=format&fit=crop)', backgroundSize: 'cover', backgroundPosition: 'center' } },
];


const PlaceholderContent = ({ appName, icon: IconComponent }: { appName: string; icon: React.ElementType }) => (
    <div className="p-6 text-white flex items-center justify-center h-full flex-col gap-4">
        <div className="w-16 h-16"><IconComponent /></div>
        <h2 className="text-lg font-semibold">{appName}</h2>
        <p className="text-sm opacity-80 text-center">Functionality for {appName} is not yet implemented.</p>
    </div>
);

const PhotosContent = () => <PlaceholderContent appName="Photos" icon={PhotosIcon} />;
const MusicContent = () => <PlaceholderContent appName="Music" icon={MusicIcon} />;
const CalendarContent = () => <PlaceholderContent appName="Calendar" icon={CalendarIcon} />;

const SettingsContent = ({ currentWallpaperId, onWallpaperChange }: { currentWallpaperId: string, onWallpaperChange: (id: string) => void}) => {
    return (
        <div className="p-4 text-white h-full overflow-y-auto">
            <h2 className="text-lg font-semibold mb-4 px-2">Desktop Wallpaper</h2>
            <div className="grid grid-cols-3 gap-4">
                {wallpapers.map(wallpaper => (
                    <div key={wallpaper.id} onClick={() => onWallpaperChange(wallpaper.id)} className="cursor-pointer group">
                        <div
                            className={`aspect-video rounded-md border-2 transition-all ${currentWallpaperId === wallpaper.id ? 'border-blue-500' : 'border-transparent group-hover:border-white/50'}`}
                            style={wallpaper.style}
                        ></div>
                        <p className="text-xs text-center mt-1 opacity-80">{wallpaper.name}</p>
                    </div>
                ))}
            </div>
        </div>
    );
};


const SystemProfilerContent = () => (
    <div className="p-4 text-white font-mono text-xs overflow-auto h-full bg-black/20">
        <p className="font-bold mb-2">--- System Information ---</p>
        <p><span className="font-semibold">OS:</span> macOS Tahoe</p>
        <p><span className="font-semibold">Boot Mode:</span> UEFI (macOS always uses EFI)</p>
        <div className="mt-2">
            <p className="font-semibold">System Info:</p>
            <div className="pl-4">
                <p><span className="w-32 inline-block">Model Identifier:</span> Tahoe1,1</p>
                <p><span className="w-32 inline-block">Processor Name:</span> Summit M4 Pro</p>
                <p><span className="w-32 inline-block">Memory:</span> 32 GB</p>
                <p><span className="w-32 inline-block">EFI Firmware:</span> T100.0.1.B2</p>
                <p><span className="w-32 inline-block">SMC Version:</span> 2.55f2</p>
            </div>
        </div>
    </div>
);


const CodeEditorContent = () => (
    <div className="p-4 text-white font-mono text-xs overflow-auto h-full bg-black/20">
        <p><span className="text-purple-400"># Polyglot Backend Architecture</span></p>
        <br />
        <p><span className="text-sky-400">## C++ Services (Performance-critical)</span></p>
        <p><span className="text-gray-400">- Real-time Data Processing</span></p>
        <p><span className="text-gray-400">- Gaming Engine Components</span></p>
        <p><span className="text-gray-400">- Scientific Computing</span></p>
        <br />
        <p><span className="text-yellow-400">## Python Services (Data &amp; AI)</span></p>
        <p><span className="text-gray-400">- Data Science &amp; Machine Learning</span></p>
        <p><span className="text-gray-400">- Automation &amp; Scripting</span></p>
        <p><span className="text-gray-400">- Business Logic Orchestration</span></p>
        <br />
        <p><span className="text-green-400">## JavaScript (Node.js) Services</span></p>
        <p><span className="text-gray-400">- API Gateway / BFF</span></p>
        <p><span className="text-gray-400">- Real-time Communication (WebSockets)</span></p>
        <p><span className="text-gray-400">- User Authentication &amp; Authorization</span></p>
    </div>
);

const HybridSimContent = () => {
    const [simState, setSimState] = useState({
        isRunning: false,
        speed: 0,
        acceleration: 0,
        engineRPM: 800,
        batteryCharge: 80,
        fuelLevel: 50,
        distance: 0,
        totalPower: 0,
        timestamp: 0,
        mode: 'Eco',
    });
    const [controls, setControls] = useState({ throttle: 0, braking: 0, motorAssist: 0.5 });
    // FIX: Replaced NodeJS.Timeout with number for browser compatibility as setInterval returns a number in the browser.
    const simIntervalRef = useRef<number | null>(null);

    const startSim = () => {
        if (simIntervalRef.current) return;
        setSimState(s => ({ ...s, isRunning: true }));
        simIntervalRef.current = window.setInterval(() => {
            setSimState(s => {
                const { throttle, braking } = controls;
                const modeMultipliers = { Eco: 0.8, Sport: 1.2, Efficiency: 1.0 };
                const multiplier = modeMultipliers[s.mode] || 1.0;
                
                let newAcceleration = (throttle * 5 * multiplier) - (braking * 10) - (s.speed * 0.01);
                let newSpeed = Math.max(0, s.speed + newAcceleration * 0.1);
                
                let newBattery = s.batteryCharge - (throttle * 0.02 * multiplier) + (braking * newSpeed > 10 ? 0.01 : 0);
                let newFuel = s.fuelLevel - (throttle * 0.01);

                return {
                    ...s,
                    timestamp: s.timestamp + 0.1,
                    speed: newSpeed,
                    acceleration: newAcceleration,
                    engineRPM: Math.max(800, 800 + newSpeed * 30),
                    batteryCharge: Math.max(0, Math.min(100, newBattery)),
                    fuelLevel: Math.max(0, newFuel),
                    distance: s.distance + (newSpeed / 3600) * 100,
                    totalPower: (newSpeed * throttle * multiplier) / 5,
                };
            });
        }, 100);
    };

    const stopSim = () => {
        if (simIntervalRef.current) clearInterval(simIntervalRef.current);
        simIntervalRef.current = null;
        setSimState(s => ({ ...s, isRunning: false, acceleration: 0, totalPower: 0 }));
    };

    const resetSim = () => {
        stopSim();
        setSimState({
            isRunning: false, speed: 0, acceleration: 0, engineRPM: 800, batteryCharge: 80,
            fuelLevel: 50, distance: 0, totalPower: 0, timestamp: 0, mode: 'Eco'
        });
        setControls({ throttle: 0, braking: 0, motorAssist: 0.5 });
    };

    useEffect(() => {
        return () => {
            if (simIntervalRef.current) clearInterval(simIntervalRef.current);
        };
    }, []);

    return (
        <div className="p-4 bg-black/20 text-white h-full overflow-y-auto text-sm">
            <h3 className="text-lg font-bold text-center mb-2">Hybrid System Simulation</h3>
            <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
                <div className="bg-white/10 p-3 rounded-lg">
                    <h4 className="font-semibold mb-2">Vehicle State</h4>
                    <p>Speed: {simState.speed.toFixed(1)} km/h</p>
                    <p>Engine RPM: {simState.engineRPM.toFixed(0)}</p>
                </div>
                <div className="bg-white/10 p-3 rounded-lg">
                    <h4 className="font-semibold mb-2">Energy Status</h4>
                    <p>Battery: {simState.batteryCharge.toFixed(1)}%</p>
                    <p>Fuel: {simState.fuelLevel.toFixed(1)} L</p>
                </div>
                <div className="bg-white/10 p-3 rounded-lg">
                    <h4 className="font-semibold mb-2">Performance</h4>
                    <p>Power: {simState.totalPower.toFixed(1)} kW</p>
                    <p>Distance: {simState.distance.toFixed(2)} km</p>
                </div>
            </div>
             <div className="bg-white/10 p-3 rounded-lg mb-4">
                <h4 className="font-semibold mb-2">Control Mode</h4>
                <div className="flex gap-2">
                    {['Eco', 'Sport', 'Efficiency'].map(m => (
                        <button key={m} onClick={() => setSimState(s => ({...s, mode: m}))} className={`flex-1 p-2 rounded text-xs transition-colors ${simState.mode === m ? 'bg-blue-600' : 'bg-gray-700 hover:bg-gray-600'}`}>{m}</button>
                    ))}
                </div>
            </div>
            <div className="bg-white/10 p-3 rounded-lg">
                <h4 className="font-semibold mb-2">Manual Controls</h4>
                <div>
                    <label>Throttle: {controls.throttle * 100}%</label>
                    <input type="range" min="0" max="1" step="0.01" value={controls.throttle} onChange={e => setControls(c => ({...c, throttle: parseFloat(e.target.value)}))} className="w-full" />
                </div>
                <div className="mt-2">
                    <label>Braking: {controls.braking * 100}%</label>
                    <input type="range" min="0" max="1" step="0.01" value={controls.braking} onChange={e => setControls(c => ({...c, braking: parseFloat(e.target.value)}))} className="w-full" />
                </div>
                 <div className="flex gap-2 mt-4">
                    <button onClick={simState.isRunning ? stopSim : startSim} className="flex-1 p-2 rounded bg-green-600 hover:bg-green-700">{simState.isRunning ? 'Stop' : 'Start'} Sim</button>
                    <button onClick={resetSim} className="flex-1 p-2 rounded bg-gray-700 hover:bg-gray-600">Reset</button>
                </div>
            </div>
        </div>
    );
};

const SandboxContent = () => {
    const canvasRef = useRef<HTMLCanvasElement>(null);
    const ballsRef = useRef<any[]>([]);
    const animationFrameRef = useRef<number | null>(null);

    const Ball = (x, y, canvas) => ({
        x, y,
        vx: (Math.random() - 0.5) * 4,
        vy: (Math.random() - 0.5) * 4,
        radius: 10 + Math.random() * 10,
        color: `hsl(${Math.random() * 360}, 70%, 70%)`,
        update() {
            this.vy += 0.2; // Gravity
            this.x += this.vx;
            this.y += this.vy;
            if (this.y + this.radius > canvas.height) {
                this.y = canvas.height - this.radius;
                this.vy *= -0.8; // Bounce
            }
            if (this.x + this.radius > canvas.width || this.x - this.radius < 0) {
                this.vx *= -1;
            }
        },
        draw(ctx) {
            ctx.beginPath();
            ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
            ctx.fillStyle = this.color;
            ctx.fill();
            ctx.closePath();
        }
    });

    const handleClick = (e: React.MouseEvent<HTMLCanvasElement>) => {
        const canvas = canvasRef.current;
        if (!canvas) return;
        const rect = canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        for (let i = 0; i < 5; i++) {
           ballsRef.current.push(Ball(x, y, canvas));
        }
    };
    
    const clearCanvas = () => {
       ballsRef.current = [];
    };

    useEffect(() => {
        const canvas = canvasRef.current;
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        if (!ctx) return;
        const animate = () => {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ballsRef.current.forEach(ball => {
                ball.update();
                ball.draw(ctx);
            });
            animationFrameRef.current = requestAnimationFrame(animate);
        };
        animate();
        return () => {
            if (animationFrameRef.current) cancelAnimationFrame(animationFrameRef.current);
        }
    }, []);

    return (
      <div className="flex flex-col h-full bg-black/30">
        <canvas ref={canvasRef} onClick={handleClick} className="flex-grow w-full h-full cursor-pointer"></canvas>
        <button onClick={clearCanvas} className="w-full bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-4 transition-colors">Clear</button>
      </div>
    );
};


const CppEngineContent = () => {
    const [simState, setSimState] = useState({
        isRunning: false,
        speed: 0,
        acceleration: 0,
        batteryCharge: 80,
        fuelLevel: 50,
    });
    // FIX: Replaced NodeJS.Timeout with number for browser compatibility as setInterval returns a number in the browser.
    const simIntervalRef = useRef<number | null>(null);
    
    const toggleSim = () => {
        if(simState.isRunning) {
            if (simIntervalRef.current) clearInterval(simIntervalRef.current);
            simIntervalRef.current = null;
        } else {
            simIntervalRef.current = window.setInterval(() => {
                setSimState(s => ({
                    ...s,
                    speed: Math.max(0, s.speed + (Math.random() - 0.4) * 5),
                    acceleration: (Math.random() - 0.5) * 2,
                    batteryCharge: Math.max(0, Math.min(100, s.batteryCharge - Math.random() * 0.1)),
                    fuelLevel: Math.max(0, s.fuelLevel - Math.random() * 0.05)
                }));
            }, 200);
        }
        setSimState(s => ({...s, isRunning: !s.isRunning}));
    };
    
    useEffect(() => {
        return () => {
            if (simIntervalRef.current) clearInterval(simIntervalRef.current);
        };
    }, []);

    return (
        <div className="p-6 text-white flex flex-col h-full">
            <h2 className="text-lg font-semibold mb-4 text-center">C++ Physics Engine</h2>
            <div className="flex-grow space-y-3">
                <p>Speed: <span className="font-mono">{simState.speed.toFixed(2)} km/h</span></p>
                <p>Acceleration: <span className="font-mono">{simState.acceleration.toFixed(2)} m/s²</span></p>
                <p>Battery Charge: <span className="font-mono">{simState.batteryCharge.toFixed(2)}%</span></p>
                <p>Fuel Level: <span className="font-mono">{simState.fuelLevel.toFixed(2)} L</span></p>
            </div>
            <button onClick={toggleSim} className="w-full bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded transition-colors">
                {simState.isRunning ? 'Stop Simulation' : 'Start Simulation'}
            </button>
        </div>
    );
};

// --- Main App Component ---

// FIX: Define types for window state to resolve 'does not exist on type unknown' errors.
interface WindowPosition {
    x: number;
    y: number;
}

interface WindowSize {
    width: number;
    height: number;
}

interface WindowState {
    id: string;
    position: WindowPosition;
    size: WindowSize;
    isMinimized: boolean;
    isMaximized: boolean;
    isClosing: boolean;
    zIndex: number;
    prevSize: WindowSize;
    prevPos: WindowPosition;
}

// FIX: Add discriminated union type for appConfig to ensure type safety when accessing properties like 'isLink' and 'url'.
interface AppWithContentConfig {
    icon: React.ElementType;
    content: React.ElementType;
    defaultSize: { width: number; height: number };
    isLink?: false;
}

interface AppAsLinkConfig {
    icon: React.ElementType;
    isLink: true;
    url: string;
}

type AppConfigItem = AppWithContentConfig | AppAsLinkConfig;

const appConfig: Record<string, AppConfigItem> = {
    'Finder': { icon: FinderIcon, content: FinderContent, defaultSize: { width: 500, height: 400 } },
    'Weather': { icon: WeatherIcon, content: WeatherContent, defaultSize: { width: 350, height: 350 } },
    'Installer': { icon: InstallerIcon, content: InstallerContent, defaultSize: { width: 400, height: 300 } },
    'Photos': { icon: PhotosIcon, content: PhotosContent, defaultSize: { width: 400, height: 300 } },
    'Music': { icon: MusicIcon, content: MusicContent, defaultSize: { width: 400, height: 300 } },
    'Amazon Music': { icon: AmazonMusicIcon, isLink: true, url: 'https://music.amazon.com' },
    'Calendar': { icon: CalendarIcon, content: CalendarContent, defaultSize: { width: 400, height: 300 } },
    'Settings': { icon: SettingsIcon, content: SettingsContent, defaultSize: { width: 550, height: 350 } },
    'System Profiler': { icon: SystemProfilerIcon, content: SystemProfilerContent, defaultSize: { width: 450, height: 350 } },
    'Code Editor': { icon: CodeIcon, content: CodeEditorContent, defaultSize: { width: 500, height: 400 } },
    'Hybrid Sim': { icon: HybridSimIcon, content: HybridSimContent, defaultSize: { width: 600, height: 550 } },
    'Sandbox': { icon: SandboxIcon, content: SandboxContent, defaultSize: { width: 500, height: 400 } },
    'C++ Engine': { icon: CppIcon, content: CppEngineContent, defaultSize: { width: 400, height: 300 } },
};

// FIX: Add props type for Window component for type safety.
interface WindowProps {
    app: AppConfigItem;
    state: WindowState;
    onClose: () => void;
    onMinimize: () => void;
    onMaximize: () => void;
    onFocus: () => void;
    wallpaperId: string;
    setWallpaperId: (id: string) => void;
}


const Window = ({ app, state, onClose, onMinimize, onMaximize, onFocus, wallpaperId, setWallpaperId }: WindowProps) => {
    const nodeRef = useRef(null);
    
    // FIX: Type guard to ensure app has content before rendering a window.
    if (!('content' in app)) {
        return null;
    }
    const Content = app.content;
    
    const contentProps: any = {};
    if (state.id === 'Settings') {
        contentProps.currentWallpaperId = wallpaperId;
        contentProps.onWallpaperChange = setWallpaperId;
    }


    return (
        <Draggable
            nodeRef={nodeRef}
            handle=".window-header"
            position={state.position}
            onStart={onFocus}
            disabled={state.isMaximized}
        >
            <div
                ref={nodeRef}
                className={`window absolute bg-white/10 backdrop-blur-2xl rounded-lg border border-white/20 shadow-2xl overflow-hidden
                    ${state.isMaximized ? 'w-full h-full top-0 left-0 rounded-none' : ''}
                    ${state.isMinimized ? 'opacity-0 scale-50' : 'opacity-100 scale-100'}
                    ${state.isClosing ? 'opacity-0 scale-90' : ''}
                    transition-all duration-300 ease-in-out`}
                style={{
                    width: state.isMaximized ? '100%' : state.size.width,
                    height: state.isMaximized ? 'calc(100vh - 25px - 70px)' : state.size.height,
                    top: state.isMaximized ? '25px' : state.position.y,
                    left: state.isMaximized ? '0' : state.position.x,
                    zIndex: state.zIndex,
                }}
                onClick={onFocus}
            >
                <div className="window-header h-8 bg-white/10 flex items-center justify-between px-3 cursor-move">
                    <div className="flex items-center gap-2">
                        <div onClick={onClose} className="w-3 h-3 bg-red-500 rounded-full cursor-pointer"></div>
                        <div onClick={onMinimize} className="w-3 h-3 bg-yellow-500 rounded-full cursor-pointer"></div>
                        <div onClick={onMaximize} className="w-3 h-3 bg-green-500 rounded-full cursor-pointer"></div>
                    </div>
                    <div className="text-sm font-semibold flex items-center gap-2">
                        <div className="w-4 h-4"><app.icon /></div>
                        {state.id}
                    </div>
                    <div style={{ width: '60px' }}></div>
                </div>
                <div className="window-content h-[calc(100%-2rem)]">
                    <Content {...contentProps} />
                </div>
            </div>
        </Draggable>
    );
};

const App = () => {
    const [time, setTime] = useState('');
    const [date, setDate] = useState('');
    // FIX: Type the windows state to resolve property access errors on 'unknown'.
    const [windows, setWindows] = useState<Record<string, WindowState>>({});
    const [bouncingApp, setBouncingApp] = useState<string | null>(null);
    const [wallpaperId, setWallpaperId] = useState('default');

    const currentWallpaperStyle = wallpapers.find(w => w.id === wallpaperId)?.style || wallpapers[0].style;

    const updateTime = useCallback(() => {
        const now = new Date();
        setTime(now.toLocaleTimeString('en-US', { hour: 'numeric', minute: '2-digit', hour12: true }));
        setDate(now.toLocaleDateString('en-US', { weekday: 'long', month: 'long', day: 'numeric' }));
    }, []);

    useEffect(() => {
        updateTime();
        const interval = setInterval(updateTime, 1000);
        return () => clearInterval(interval);
    }, [updateTime]);

    const openApp = (id: string) => {
        setBouncingApp(id);
        setTimeout(() => setBouncingApp(null), 500);
        
        const config = appConfig[id];
// FIX: Restructure to handle app links and windowed apps in separate branches.
// This ensures that `config` is correctly typed when creating a new window,
// resolving errors when accessing `config.defaultSize`.
        if ('content' in config) {
            if (windows[id] && windows[id].isMinimized) {
                restoreWindow(id);
                return;
            }
    
            if (windows[id]) {
                focusWindow(id);
                return;
            }
    
            const newZIndex = Math.max(0, ...Object.values(windows).map(w => w.zIndex)) + 1;
            setWindows(prev => ({
                ...prev,
                [id]: {
                    id,
                    position: { x: 100 + Object.keys(prev).length * 20, y: 100 + Object.keys(prev).length * 20 },
                    size: config.defaultSize,
                    isMinimized: false,
                    isMaximized: false,
                    isClosing: false,
                    zIndex: newZIndex,
                    prevSize: config.defaultSize,
                    prevPos: { x: 100 + Object.keys(prev).length * 20, y: 100 + Object.keys(prev).length * 20 },
                },
            }));
        } else {
            window.open(config.url, '_blank', 'noopener,noreferrer');
        }
    };
    
    const closeWindow = (id: string) => {
        setWindows(prev => {
            const newWindows = { ...prev };
            if (newWindows[id]) {
                newWindows[id].isClosing = true;
            }
            return newWindows;
        });
        setTimeout(() => {
            setWindows(prev => {
                const newWindows = { ...prev };
                delete newWindows[id];
                return newWindows;
            });
        }, 300);
    };

    const focusWindow = (id: string) => {
        const newZIndex = Math.max(0, ...Object.values(windows).map(w => w.zIndex)) + 1;
        setWindows(prev => ({
            ...prev,
            [id]: { ...prev[id], zIndex: newZIndex }
        }));
    };
    
    const minimizeWindow = (id: string) => {
        setWindows(prev => ({
            ...prev,
            [id]: { ...prev[id], isMinimized: true }
        }));
    };
    
    const restoreWindow = (id: string) => {
        focusWindow(id);
        setWindows(prev => ({
            ...prev,
            [id]: { ...prev[id], isMinimized: false }
        }));
    };

    const toggleMaximize = (id: string) => {
        setWindows(prev => {
            const win = prev[id];
            if (win.isMaximized) {
                return {
                    ...prev,
                    [id]: { ...win, isMaximized: false, size: win.prevSize, position: win.prevPos }
                };
            } else {
                return {
                    ...prev,
                    [id]: { ...win, isMaximized: true, prevSize: win.size, prevPos: win.position }
                };
            }
        });
    };

    return (
        <div className="desktop h-screen w-screen overflow-hidden relative text-white">
            <div className="tahoe-background absolute inset-0 bg-gradient-to-br from-indigo-900 to-blue-900 transition-all duration-500" style={currentWallpaperStyle}></div>
            
            <div className="menu-bar h-[25px] bg-white/15 backdrop-blur-2xl border-b border-white/10 flex justify-between items-center px-5 text-sm font-medium">
                <div className="flex gap-5">
                    <div></div>
                    <div>Finder</div>
                    <div>File</div>
                    <div>Edit</div>
                </div>
                <div>{time}</div>
            </div>

            <div className="time-display absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 text-center pointer-events-none">
                <div className="text-8xl font-thin tracking-tighter">{time}</div>
                <div className="text-2xl opacity-80 font-light">{date}</div>
            </div>

            {Object.values(windows).map(win => (
                <Window
                    key={win.id}
                    app={appConfig[win.id]}
                    state={win}
                    onClose={() => closeWindow(win.id)}
                    onMinimize={() => minimizeWindow(win.id)}
                    onMaximize={() => toggleMaximize(win.id)}
                    onFocus={() => focusWindow(win.id)}
                    wallpaperId={wallpaperId}
                    setWallpaperId={setWallpaperId}
                />
            ))}

            <div className="dock absolute bottom-2.5 left-1/2 -translate-x-1/2 bg-white/15 backdrop-blur-2xl rounded-2xl p-2 flex gap-2 border border-white/10">
                {Object.entries(appConfig).map(([id, config]) => (
                    <div
                        key={id}
                        onClick={() => openApp(id)}
                        className={`w-14 h-14 rounded-xl flex items-center justify-center cursor-pointer transition-all duration-200 relative
                            ${bouncingApp === id ? 'animate-bounce' : 'hover:scale-125'}`}
                    >
                        <div className="w-12 h-12 p-1"><config.icon /></div>
                        {!config.isLink && windows[id] && !windows[id].isClosing && <div className="absolute bottom-[-6px] w-1.5 h-1.5 bg-white rounded-full"></div>}
                    </div>
                ))}
            </div>
        </div>
    );
};

export default App;
