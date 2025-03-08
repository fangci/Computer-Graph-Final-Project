# Rendering an Island with Dynamic Changing Landscape

This project presents a **realistic island scene** with dynamically changing landscapes, integrating **terrain, ocean, sky, vegetation, and sunlight variations** to simulate a natural environment.

![預覽](/images/project.png)

## 🌍 **Project Overview**
This project creates a **natural island environment** where elements change over time:
- **Terrain**: Procedurally generated using **Perlin Noise**, with different textures based on elevation.
![預覽](/images/island.png)
- **Ocean**: **FFT-based waves** simulate realistic water movement and reflections.
![預覽](/images/sea.png)
- **Sky**: Implemented using **Skybox**, with a slow rotation for enhanced realism.
![預覽](/images/sky.png)
- **Vegetation**: Randomly placed plants that adapt to terrain slopes.
![預覽](/images/plant.png)
- **Sunlight**: Dynamic lighting simulating sunrise, noon, sunset, and night.
![預覽](/images/light.png)

---

## ✨ **Features**

### **1️⃣ Terrain Generation**
- **Perlin Noise** generates **random landscapes** with natural elevation variations.
- **Texture variations based on altitude**:
  - **Low altitude** → Moss texture (wet, lush vegetation).
  - **High altitude** → Rock texture (dry, rugged terrain).
  - **Mid-altitude** → Smooth blending of moss and rock.

### **2️⃣ Ocean Simulation**
- **FFT-based wave generation**, simulating realistic water movement.
- **Reflective surface and transparency effects** based on depth.
- **Dynamic wave highlights** based on light intensity and direction.

### **3️⃣ Sky and Atmosphere**
- **Skybox** implementation for a seamless 360° environment.
- **Slow rotation for realism**.
- **Sky color changes throughout the day** (warm tones at sunrise and sunset).

### **4️⃣ Vegetation & Nature**
- **Randomly placed grass** adapts to terrain slopes.
- **Simulated wind effects** cause subtle plant movement.

### **5️⃣ Sunlight and Dynamic Lighting**
- **Directional light simulation** for realistic sun movement.
- **Light intensity & color change throughout the day**:
  - **Morning & Evening** → Warm orange hue.
  - **Noon** → Bright white light.
  - **Night** → Low ambient lighting.
![預覽](/images/light02.png)
---

## 🛠 **Technical Implementation**
- **Shaders**: Controls terrain blending, lighting, and reflections.
- **FFT Wave Simulation**: Creates dynamic water movement.
- **Normal Mapping**: Enhances terrain and water realism.
- **OpenGL & GLSL**: Used for real-time rendering.

---

## 🚀 **How to Run**
### **1️⃣ Setup**
- **Recommended IDE: Visual Studio**.
- Install OpenGL and graphics libraries.

### **2️⃣ Clone Repository**
```bash
git clone https://github.com/your-repo/island-rendering.git
```

3️⃣ Run in Visual Studio
Open Visual Studio.
Create a new project and import files.
Build and Run.

🌴 Explore the dynamic island world! 🏝️🌊☀️
