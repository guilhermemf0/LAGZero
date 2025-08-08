document.addEventListener("DOMContentLoaded", () => {
  // Lógica das partículas de fundo
  const particlesContainer = document.querySelector(".background-particles");

  function createParticle() {
    const particle = document.createElement("div");
    particle.classList.add("particle");
    particlesContainer.appendChild(particle);

    const size = Math.random() * 8 + 3; // Tamanho entre 3px e 11px
    const animationDuration = Math.random() * 10 + 8; // Duração da animação entre 8s e 18s
    const animationDelay = -Math.random() * 10; // Atraso negativo para iniciar em diferentes pontos
    const startLeft = Math.random() * 100; // Posição horizontal inicial aleatória
    const startTop = 100 + Math.random() * 50; // Começa abaixo da tela
    const xDrift = Math.random() * 400 - 200; // Desvio horizontal entre -200px e 200px

    particle.style.width = `${size}px`;
    particle.style.height = `${size}px`;
    particle.style.left = `${startLeft}%`;
    particle.style.top = `${startTop}vh`;
    particle.style.animationDuration = `${animationDuration}s`;
    particle.style.animationDelay = `${animationDelay}s`;
    particle.style.setProperty("--x-drift", `${xDrift}px`); // Define uma variável CSS para o desvio horizontal
  }

  const numberOfParticles = 120; // Aumentado o número de partículas para um efeito mais denso
  for (let i = 0; i < numberOfParticles; i++) {
    createParticle();
  }

  // Lógica do Modal "Sobre"
  const aboutLink = document.getElementById("about-link");
  const aboutModal = document.getElementById("about-modal");
  const closeButton = aboutModal.querySelector(".close-button");

  // Abre o modal
  aboutLink.addEventListener("click", (e) => {
    e.preventDefault(); // Impede o comportamento padrão do link
    aboutModal.classList.add("open");
  });

  // Fecha o modal ao clicar no botão de fechar ou no overlay
  closeButton.addEventListener("click", () => {
    aboutModal.classList.remove("open");
  });

  aboutModal.addEventListener("click", (e) => {
    if (e.target === aboutModal) {
      // Fecha apenas se clicar no overlay, não no conteúdo do modal
      aboutModal.classList.remove("open");
    }
  });
});
