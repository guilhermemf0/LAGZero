document.addEventListener("DOMContentLoaded", () => {
  const preloader = document.getElementById("preloader");
  const mainContent = document.getElementById("main-content");
  const header = document.querySelector(".header");

  setTimeout(() => {
    preloader.classList.add("loaded");
    mainContent.classList.add("visible");
    header.style.transform = "translateY(0)";
  }, 2500);

  const auroraLayer = document.querySelector(".aurora-layer");
  window.addEventListener("mousemove", (e) => {
    auroraLayer.style.setProperty("--mouse-x", `${e.clientX}px`);
    auroraLayer.style.setProperty("--mouse-y", `${e.clientY}px`);
  });

  let scene, camera, renderer, particles;
  const container = document.getElementById("particles-container");

  function initThreeJS() {
    if (!container || !window.THREE) return;

    scene = new THREE.Scene();
    camera = new THREE.PerspectiveCamera(
      75,
      window.innerWidth / window.innerHeight,
      0.1,
      1000
    );
    camera.position.z = 500;

    renderer = new THREE.WebGLRenderer({ alpha: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);
    container.appendChild(renderer.domElement);

    const particleCount = 1500;
    const positions = new Float32Array(particleCount * 3);
    const velocities = new Float32Array(particleCount * 3);

    for (let i = 0; i < particleCount; i++) {
      positions[i * 3] = (Math.random() - 0.5) * 1000;
      positions[i * 3 + 1] = (Math.random() - 0.5) * 1000;
      positions[i * 3 + 2] = (Math.random() - 0.5) * 1000;

      velocities[i * 3] = 0;
      velocities[i * 3 + 1] = Math.random() * 0.2 + 0.05;
      velocities[i * 3 + 2] = 0;
    }

    const geometry = new THREE.BufferGeometry();
    geometry.setAttribute("position", new THREE.BufferAttribute(positions, 3));
    geometry.setAttribute("velocity", new THREE.BufferAttribute(velocities, 3));

    const material = new THREE.PointsMaterial({
      color: 0x0085ff,
      size: 1.0,
      transparent: true,
      opacity: 0.6,
      blending: THREE.AdditiveBlending,
    });

    particles = new THREE.Points(geometry, material);
    scene.add(particles);

    window.addEventListener("resize", onWindowResize, false);
  }

  function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
  }

  function animate() {
    if (!particles) return;
    requestAnimationFrame(animate);

    const positions = particles.geometry.attributes.position.array;
    const velocities = particles.geometry.attributes.velocity.array;

    for (let i = 0; i < positions.length / 3; i++) {
      positions[i * 3 + 1] += velocities[i * 3 + 1];

      if (positions[i * 3 + 1] > 500) {
        positions[i * 3 + 1] = -500;
      }
    }

    particles.geometry.attributes.position.needsUpdate = true;
    scene.rotation.y += 0.0001;
    renderer.render(scene, camera);
  }

  initThreeJS();
  animate();

  const featureItems = document.querySelectorAll(".feature-item");
  const featureVisuals = document.querySelectorAll(".feature-visual-content");

  featureItems.forEach((item) => {
    item.addEventListener("click", () => {
      featureItems.forEach((i) => i.classList.remove("active"));
      featureVisuals.forEach((v) => v.classList.remove("active"));

      item.classList.add("active");
      const targetId = item.dataset.target;
      document.getElementById(targetId).classList.add("active");
    });
  });

  const sections = document.querySelectorAll(".content-section");
  const observer = new IntersectionObserver(
    (entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting) {
          entry.target.classList.add("is-visible");
          observer.unobserve(entry.target);
        }
      });
    },
    {
      threshold: 0.15,
    }
  );

  sections.forEach((section) => {
    observer.observe(section);
  });

  const aboutLink = document.getElementById("about-link");
  const aboutModal = document.getElementById("about-modal");
  const closeButton = aboutModal.querySelector(".close-button");

  const openModal = () => aboutModal.classList.add("open");
  const closeModal = () => aboutModal.classList.remove("open");

  aboutLink.addEventListener("click", (e) => {
    e.preventDefault();
    openModal();
  });
  closeButton.addEventListener("click", closeModal);
  aboutModal.addEventListener(
    "click",
    (e) => e.target === aboutModal && closeModal()
  );
  document.addEventListener(
    "keydown",
    (e) => e.key === "Escape" && closeModal()
  );
});
