#include "formatter/html/components/help_tooltip.hpp"

namespace mem_analyzer {

std::string help_tooltip_css() {
    return R"HTML(/* ========== 添加 help-icon 样式 ========== */
.help-icon {
    display: inline-block;
    width: 18px;
    height: 18px;
    line-height: 18px;
    text-align: center;
    border-radius: 50%;
    background-color: #ccc;
    color: #fff;
    font-weight: bold;
    font-size: 14px;
    cursor: help;
    margin-left: 6px;
    position: relative;
    user-select: none;
}

.help-tooltip {
    position: fixed;
    z-index: 1000;
    background: #333;
    color: #fff;
    padding: 10px 14px;
    border-radius: 6px;
    font-size: 15px;
    line-height: 1.5;
    white-space: pre-wrap;        /* 保留 \n 换行，超长内容自动折行 */
    overflow-wrap: break-word;    /* 单个长词/长 token 过长时允许断行 */
    max-width: min(72vw, 560px);  /* 限制宽度，避免超出视口 */
    text-align: left;
    font-weight: normal;
    box-shadow: 0 4px 12px rgba(0,0,0,0.3);
    opacity: 0;
    pointer-events: none;
    transition: opacity 0.2s ease;
}

.help-tooltip.show {
    opacity: 1;
}

.help-tooltip::before {
    content: '';
    position: absolute;
    left: var(--arrow-x, 50%);
    transform: translateX(-50%);
    border: 6px solid transparent;
}

.help-tooltip.above::before {
    bottom: -12px;
    border-top-color: #333;
}

.help-tooltip.below::before {
    top: -12px;
    border-bottom-color: #333;
})HTML";
}

std::string help_tooltip_js() {
    return R"HTML(  function initHelpTooltip() {
    var tip = document.createElement('div');
    tip.className = 'help-tooltip';
    document.body.appendChild(tip);

    var currentEl = null;
    var margin = 12;
    var gap = 8;

    function position(el) {
      var text = el.getAttribute('data-tooltip');
      if (!text) { return; }
      tip.textContent = text;
      tip.classList.add('show');

      var rect = el.getBoundingClientRect();
      var tipW = tip.offsetWidth;
      var tipH = tip.offsetHeight;

      var left = rect.left + rect.width / 2 - tipW / 2;
      var top = rect.top - tipH - gap;

      if (top < margin) {
        top = rect.bottom + gap;
        tip.classList.add('below');
        tip.classList.remove('above');
      } else {
        tip.classList.add('above');
        tip.classList.remove('below');
      }

      left = Math.max(margin, Math.min(left, window.innerWidth - tipW - margin));

      var arrowX = rect.left + rect.width / 2 - left;
      arrowX = Math.max(10, Math.min(arrowX, tipW - 10));

      tip.style.left = left + 'px';
      tip.style.top = top + 'px';
      tip.style.setProperty('--arrow-x', arrowX + 'px');
    }

    function hide() {
      tip.classList.remove('show');
      currentEl = null;
    }

    document.addEventListener('mouseover', function(e) {
      var el = e.target && e.target.closest ? e.target.closest('.help-icon') : null;
      if (el) { currentEl = el; position(el); }
    });
    document.addEventListener('mouseout', function(e) {
      var el = e.target && e.target.closest ? e.target.closest('.help-icon') : null;
      if (el) { hide(); }
    });
    window.addEventListener('resize', function() {
      if (currentEl) { position(currentEl); }
    });
    window.addEventListener('scroll', function() {
      if (currentEl) { position(currentEl); }
    }, true);
  }
  initHelpTooltip();)HTML";
}

}
